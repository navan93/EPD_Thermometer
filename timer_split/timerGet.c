#include "timer.h"
#include "cpu.h"

static volatile uint16_t mTmrHi;


uint32_t timerGet(void)
{
	union {
		struct {
			uint8_t tL;
			uint8_t tH;
			uint16_t hi;
		};
		uint32_t ret;
	} val;
	
	do {
		val.hi = mTmrHi;
		val.tH = TH0;
		val.tL = TL0;	//read order is important due ot hardware buffering
	} while (val.hi != mTmrHi || val.tH != TH0);
	
	return val.ret;
}
