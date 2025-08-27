#define __packed
#include "config.h"
#include <stdint.h>

#include "powermgt.h"
#include "printf.h"

#include "wdt.h"



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
