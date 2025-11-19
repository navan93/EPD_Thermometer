#include "uart.h"

#include "cpu.h"
#include "stdbool.h"
#include "string.h"


#include "function_signatures.h"

void uartTx(uint8_t val) {
    while (!(UARTSTA & (1 << 1)))
        ;
    UARTSTA &= ~(1 << 1);
    UARTBUF = val;
}
