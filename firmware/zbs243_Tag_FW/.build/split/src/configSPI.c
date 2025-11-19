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

void configSPI(const bool setup) {
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
