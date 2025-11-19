#include "config.h"
#include "powermgt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "asmUtil.h"
#include "board.h"
#include "cpu.h"
#include "eeprom.h"
#include "printf.h"
#include "sleep.h"
#include "timer.h"
#include "uart.h"  // for initUart
#include "wdt.h"

#if RADIO_ENABLED
uint16_t __xdata dataReqAttemptArr[POWER_SAVING_SMOOTHING] = {0};  // Holds the amount of attempts required per data_req/check-in
uint8_t __xdata dataReqAttemptArrayIndex = 0;
uint8_t __xdata dataReqLastAttempt = 0;
uint16_t __xdata nextCheckInFromAP = 0;
uint8_t __xdata wakeUpReason = 0;
uint8_t __xdata scanAttempts = 0;
#endif

int8_t __xdata temperature = 0;
uint16_t __xdata batteryVoltage = 2600;
bool __xdata lowBattery = false;

uint8_t __xdata capabilities = 0;

bool __xdata spiActive = false;
bool __xdata uartActive = false;
bool __xdata eepromActive = false;
bool __xdata i2cActive = false;
extern int8_t adcSampleTemperature(void);  // in degrees C

void setupPortsInitial() {
    P0INTEN = 0;
    P1INTEN = 0;
    P2INTEN = 0;
    P0FUNC = 0;
    P1FUNC = 0;
    P2FUNC = 0;
    P0DIR = 0xFF;
    P1DIR = 0xFF;
    P2DIR = 0xFF;
    P0PULL = 0x00;
    P1PULL = 0x00;
    P2PULL = 0x00;
}

#if SPI_ENABLED
static void configSPI(const bool setup) {
    if (setup == spiActive) return;
    if (setup) {
        P0FUNC |= (1 << 0) | (1 << 1) | (1 << 2);
        P0DIR |= (1 << 2);                // MISO as input
        P0DIR &= ~((1 << 0) | (1 << 1));  // CLK and MOSI as output
        P0PULL |= (1 << 2);
        spiInit();
        wdtOn();
    } else {
        P0FUNC &= ~((1 << 0) | (1 << 1) | (1 << 2));
        P0DIR |= (1 << 0) | (1 << 1) | (1 << 2);
        P0PULL &= ~(1 << 2);
        uint8_t bcp;
        CLKEN &= ~(0x08);
        bcp = CFGPAGE;
        CFGPAGE = 4;
        SPIENA &= ~(0x81);
        CFGPAGE = bcp;
    }
    spiActive = setup;
}
#endif

#if UART_ENABLED
static void configUART(const bool setup) {
    if (uartActive == setup) return;
    if (setup) {
        P0FUNC |= (1 << 6);
        P0DIR &= ~(1 << 6);
        uartInit();
    } else {
        P0DIR |= (1 << 6);
        P0FUNC &= ~(1 << 6);
        CLKEN &= ~(0x20);
    }
    uartActive = setup;
}
#endif

#if EEPROM_ENABLED
static void configEEPROM(const bool setup) {
    if (setup == eepromActive) return;
    if (setup) {
#ifdef EXTRA_EEPROM_LINES
        P1FUNC &= ~(1 << 1) | (1 << 2) | (1 << 6);
        P1DIR &= ~(1 << 1) | (1 << 2) | (1 << 6);
        P1_6 = 1;
        P1_2 = 1;
#else
        P1FUNC &= ~(1 << 1);
        P1DIR &= ~(1 << 1);
#endif
        if (!eepromInit()) {
            powerDown(INIT_RADIO);
            powerUp(INIT_EPD);
            showNoEEPROM();
            powerDown(INIT_EEPROM | INIT_EPD);
            doSleep(-1);
            wdtDeviceReset();
        }
    } else {
        P1DIR |= (1 << 1);
    }
    setup == eepromActive;  // wtf, this does nothing.
}
#endif // EEPROM_ENABLED

#if I2C_ENABLED
static void configI2C(const bool setup) {
    if (setup == i2cActive) return;
    if (setup) {
        P1DIR &= ~(1 << 6);
        P1_6 = 1;
        timerDelay(13330);
        P1FUNC |= (1 << 4) | (1 << 5);
        P1PULL |= (1 << 4) | (1 << 5);
        i2cInit();
        // i2cCheckDevice(0x50);  // first transaction after init fails, this makes sure everything is ready for the first transaction
    } else {
        P1DIR |= (1 << 6);
        P1_6 = 0;
        P1FUNC &= ~((1 << 4) | (1 << 5));
        P1PULL &= ~((1 << 4) | (1 << 5));
        CLKEN &= ~0x10;
        IEN1 &= ~4;
    }
    i2cActive = setup;
}
#endif

void powerUp(const uint8_t parts)
{
    if (parts & INIT_BASE) {
        clockingAndIntsInit();
        timerInit();
        irqsOn();
        wdtOn();
        wdt10s();
    }

#if EPD_ENABLED && SPI_ENABLED
    if (parts & INIT_EPD) {
        configSPI(true);
        epdConfigGPIO(true);
        epdSetup();
    }

    if (parts & INIT_EPD_VOLTREADING) {
        epdConfigGPIO(true);
        configSPI(true);
        batteryVoltage = epdGetBattery();
        if (batteryVoltage < tagSettings.batLowVoltage) {
            lowBattery = true;
        } else {
            lowBattery = false;
        }
        configSPI(false);
        epdConfigGPIO(false);
    }
#endif // EPD_ENABLED

#if UART_ENABLED
    if (parts & INIT_UART) {
        configUART(true);
    }
#endif

#if EEPROM_ENABLED
    if (parts & INIT_EEPROM) {
        configSPI(true);
        configEEPROM(true);
    }
#endif // EEPROM_ENABLED

#if BUILT_IN_TEMPERATURE_ENABLED
    if (parts & INIT_TEMPREADING) {
        temperature = adcSampleTemperature();
    }
#endif // BUILT_IN_TEMPERATURE_ENABLED
#if RADIO_ENABLED
    if (parts & INIT_RADIO) {
        radioInit();
        radioRxFilterCfg(mSelfMac, 0x10000, PROTO_PAN_ID);
        radioSetTxPower(10);
        if (currentChannel >= 11 && currentChannel <= 27) {
            radioSetChannel(currentChannel);
        } else {
            radioSetChannel(RADIO_FIRST_CHANNEL);
        }
    }
#endif
#if I2C_ENABLED
    if (parts & INIT_I2C) {
        configI2C(true);
    }
#endif
}

void powerDown(const uint8_t parts)
{
#if UART_ENABLED
    if (parts & INIT_UART) {
        configUART(false);
    }
#endif
#if RADIO_ENABLED
    if (parts & INIT_RADIO) {  // warning; this also touches some stuff about the EEPROM, apparently. Re-init EEPROM afterwards
        radioRxEnable(false, true);
        RADIO_IRQ4_pending = 0;
        UNK_C1 &= ~0x81;
        TCON &= ~0x20;
        uint8_t __xdata cfgPg = CFGPAGE;
        CFGPAGE = 4;
        RADIO_command = 0xCA;
        RADIO_command = 0xC5;
        CFGPAGE = cfgPg;
    }
#endif // RADIO_ENABLED

#if EEPROM_ENABLED
    if (parts & INIT_EEPROM) {
        eepromDeepPowerDown();
        eepromPrvDeselect();
        configEEPROM(false);
    }
#endif // EEPROM_ENABLED

#if EPD_ENABLED
    if (parts & INIT_EPD) {
        epdConfigGPIO(true);
        epdEnterSleep();
        epdConfigGPIO(false);
    }
#endif // EPD_ENABLED
#if EPD_ENABLED || EEPROM_ENABLED
    if (!eepromActive && !epdGPIOActive) {
        configSPI(false);
    }
#endif // EPD_ENABLED || EEPROM_ENABLED
#if I2C_ENABLED
    if (parts & INIT_I2C) {
        configI2C(false);
    }
#endif
}

void doSleep(const uint32_t __xdata t) {
    P0FUNC = 0;
    P1FUNC = 0;
    P2FUNC = 0;

    P0DIR = 1;
    P0 = 0;
    P0PULL = 1;

    P1DIR = 0x86;
    P1PULL = 0x86;

    P2DIR = 7;
    P2 = 0;
    P2PULL = 5;

    spiActive = false;
    uartActive = false;
    eepromActive = false;

#ifdef ISDEBUGBUILD
    capabilities |= CAPABILITY_HAS_WAKE_BUTTON;
#endif

    // if (capabilities & CAPABILITY_HAS_WAKE_BUTTON) {
    //     // Button setup on TEST pin 1.0 (input pullup)
    //     P1FUNC &= ~(1 << 0);
    //     P1DIR |= (1 << 0);
    //     P1PULL |= (1 << 0);
    //     P1LVLSEL |= (1 << 0);
    //     P1INTEN |= (1 << 0);
    //     P1CHSTA &= ~(1 << 0);

    //     // Button setup on RXD pin 0.7 (input pullup)
    //     P0FUNC &= ~(1 << 7);
    //     P0DIR |= (1 << 7);
    //     P0PULL |= (1 << 7);
    //     P0LVLSEL |= (1 << 7);
    //     P0INTEN |= (1 << 7);
    //     P0CHSTA &= ~(1 << 7);
    // }

#ifdef ENABLE_GPIO_WAKE
    // enable wake on pin 0.2 (MISO)
    P0FUNC &= ~(1 << 3);
    P0DIR |= (1 << 3);
    P0PULL |= (1 << 3);
    P0LVLSEL |= (1 << 3);
    P0INTEN |= (1 << 3);
    P0CHSTA &= ~(1 << 3);
#endif

#if RADIO_ENABLED
    if (tagSettings.enableRFWake) {
        //  enabled RF wake, adds a little extra energy draw!
        RADIO_RadioPowerCtl &= 0xFB;
    }
#endif // RADIO_ENABLED

    // sleepy time
    sleepForMsec(t);
    P1INTEN = 0;
    P0INTEN = 0;

#if RADIO_ENABLED
    switch (RADIO_Wake_Reason) {
        case RADIO_WAKE_REASON_RF:
            wakeUpReason = WAKEUP_REASON_RF;
            break;
        case RADIO_WAKE_REASON_EXT:
            if ((P1CHSTA & (1 << 0)) && (capabilities & CAPABILITY_HAS_WAKE_BUTTON)) {
                wakeUpReason = WAKEUP_REASON_BUTTON1;
                P1CHSTA &= ~(1 << 0);
            }
            if ((P0CHSTA & (1 << 7)) && (capabilities & CAPABILITY_HAS_WAKE_BUTTON)) {
                wakeUpReason = WAKEUP_REASON_BUTTON2;
                P0CHSTA &= ~(1 << 7);
            }
            if ((P1CHSTA & (1 << 3)) && (capabilities & CAPABILITY_NFC_WAKE)) {
                wakeUpReason = WAKEUP_REASON_NFC;
                P1CHSTA &= ~(1 << 3);
            }
#ifdef ENABLE_GPIO_WAKE
            if (P0CHSTA & (1 << 3)) {
                wakeUpReason = WAKEUP_REASON_GPIO;
                P0CHSTA &= ~(1 << 3);
            }
#endif
            break;
        case RADIO_WAKE_REASON_TIMER:
            // this stops the compiler from whining about a conditional flow optimization
            wakeUpReason = wakeUpReason;
            break;
    }
#endif // RADIO_ENABLED
}

void doVoltageReading() {
    powerUp(INIT_RADIO);  // load down the battery using the radio to get a good voltage reading
    powerUp(INIT_EPD_VOLTREADING | INIT_TEMPREADING);
    powerDown(INIT_RADIO);
}

#if RADIO_ENABLED
uint32_t getNextScanSleep(const bool increment) {
    if (increment) {
        if (scanAttempts < 255)
            scanAttempts++;
    }

    if (scanAttempts < INTERVAL_1_ATTEMPTS) {
        return INTERVAL_1_TIME;
    } else if (scanAttempts < (INTERVAL_1_ATTEMPTS + INTERVAL_2_ATTEMPTS)) {
        return INTERVAL_2_TIME;
    } else {
        return INTERVAL_3_TIME;
    }
}

void addAverageValue() {
    uint16_t __xdata curval = INTERVAL_AT_MAX_ATTEMPTS - INTERVAL_BASE;
    curval *= dataReqLastAttempt;
    curval /= DATA_REQ_MAX_ATTEMPTS;
    curval += INTERVAL_BASE;
    dataReqAttemptArr[dataReqAttemptArrayIndex % POWER_SAVING_SMOOTHING] = curval;
    dataReqAttemptArrayIndex++;
}

uint16_t getNextSleep() {
    uint16_t avg = 0;
    for (uint8_t c = 0; c < POWER_SAVING_SMOOTHING; c++) {
        avg += dataReqAttemptArr[c];
    }
    avg /= POWER_SAVING_SMOOTHING;

    // check if we should sleep longer due to an override in the config
    // if (avg < tagSettings.minimumCheckInTime) return tagSettings.minimumCheckInTime;
    return avg;
}

void initPowerSaving(const uint16_t initialValue) {
    for (uint8_t c = 0; c < POWER_SAVING_SMOOTHING; c++) {
        dataReqAttemptArr[c] = initialValue;
    }
}

#endif