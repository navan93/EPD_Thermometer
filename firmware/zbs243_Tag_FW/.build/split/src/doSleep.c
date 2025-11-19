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
