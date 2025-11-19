// Function signatures extracted from timer.c

#include "timer.h"
#include "cpu.h"

static volatile uint16_t mTmrHi;


void T0_ISR(void) __interrupt (1);

uint32_t timerGet(void);

uint8_t timerGetLowBits(void);

void timerInit(void);

void timerDelay(uint32_t ticks);

