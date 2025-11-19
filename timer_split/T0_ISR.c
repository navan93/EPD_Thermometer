#include "timer.h"
#include "cpu.h"

static volatile uint16_t mTmrHi;


void T0_ISR(void) __interrupt (1)
{
	TCON &=~ 0x20;	//clear flag
	mTmrHi++;
}
