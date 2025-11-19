#include "uart.h"

#include "cpu.h"
#include "stdbool.h"
#include "string.h"


#include "function_signatures.h"

void uartInit(void) {
    // clock it up
    CLKEN |= 0x20;
    // configure baud rate
    UARTBRGH = 0x00;
#ifdef AP_FW
    // UARTBRGL = 69;  // nice. 230400 baud
    //UARTBRGL = 70;  // 79 == 200k
    IEN_UART0 = 1;
    UARTBRGL = 0x8A;  // config for 115200
#else
    UARTBRGL = 0x8A;  // config for 115200
#endif
    UARTSTA = 0x12;  // also set the "empty" bit else we wait forever for it to go up
}
