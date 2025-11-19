#include "timer.h"
#include "cpu.h"

static volatile uint16_t mTmrHi;


uint8_t timerGetLowBits(void)
{
	return TL0;
}
