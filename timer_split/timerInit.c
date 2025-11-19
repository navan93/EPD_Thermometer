#include "timer.h"
#include "cpu.h"

static volatile uint16_t mTmrHi;


void timerInit(void)
{
	//clock up timers
	CLKEN |= 0x01;
	
	
	//stop and clear int flags
	TCON &=~ 0xf0;
	
	//reset
	mTmrHi = 0;
	T0 = 0;
	
	//timer 0 in 16 bit mode, timer 1 off
	TMOD = 0x31;
	
	//start
	TCON |= 0x10;
	
	//int on
	IEN_TMR0 = 1;
}
