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
