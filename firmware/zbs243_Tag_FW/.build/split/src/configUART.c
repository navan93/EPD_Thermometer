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

void configUART(const bool setup) {
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
