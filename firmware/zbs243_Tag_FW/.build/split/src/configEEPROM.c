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

void configEEPROM(const bool setup) {
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
