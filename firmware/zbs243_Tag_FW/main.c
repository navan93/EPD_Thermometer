#define __packed
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "asmUtil.h"
#include "comms.h"  // for mLastLqi and mLastRSSI
#include "eeprom.h"
#include "i2c.h"
#include "i2cdevices.h"
#include "powermgt.h"
#include "printf.h"

#include "radio.h"
#include "screen.h"
#include "settings.h"
#include "syncedproto.h"
#include "timer.h"
#include "userinterface.h"
#include "wdt.h"

#include "flash.h"

#include "uart.h"

#include "../shared/oepl-definitions.h"
#include "../shared/oepl-proto.h"


static const uint64_t __code __at(0x008b) firmwaremagic = (0xdeadd0d0beefcafeull) + HW_TYPE;

void executeCommand(uint8_t cmd) {
    (void)cmd;
}

void main() {
    setupPortsInitial();
    powerUp(INIT_BASE | INIT_UART);
    pr("BOOTED> %04X%s\n",fwVersion, fwVersionSuffix);

    wdt30s();

    pr("Splash screen\n");
    powerUp(INIT_EPD);
    showSplashScreen();
    timerDelay(TIMER_TICKS_PER_SECOND * 4);

    pr("Update screen\n");
    powerUp(INIT_EPD);
    showApplyUpdate();
    timerDelay(TIMER_TICKS_PER_SECOND * 4);

    // this is the loop we'll stay in forever, basically.
    while (1) {
        powerUp(INIT_UART);
        pr("Hello World\n");
        wdt10s();
		doSleep(2000); // Attention always also increase the unix time!
    }
}
