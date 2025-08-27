#define __packed
#include "config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "asmUtil.h"
#include "powermgt.h"
#include "printf.h"

#include "timer.h"
#include "wdt.h"

#include "flash.h"

#include "uart.h"


void executeCommand(uint8_t cmd) {
    (void)cmd;
}

void main() {
    setupPortsInitial();
    powerUp(INIT_BASE | INIT_UART);

    wdt30s();

    // this is the loop we'll stay in forever, basically.
    while (1) {
        powerUp(INIT_UART);
        pr("Hello World\n");
        wdt10s();
		doSleep(2000); // Attention always also increase the unix time!
    }
}
