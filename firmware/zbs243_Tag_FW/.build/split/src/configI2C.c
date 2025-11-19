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

void configI2C(const bool setup) {
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
