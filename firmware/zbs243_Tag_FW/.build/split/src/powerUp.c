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
#include "uart.h"
#include "wdt.h"


#include "function_signatures.h"

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
