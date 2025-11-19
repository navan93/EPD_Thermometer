#include "timer.h"
#include "cpu.h"

static volatile uint16_t mTmrHi;


void timerDelay(uint32_t ticks)
{
	uint32_t start = timerGet();
	while (timerGet() - start <= ticks);
}
