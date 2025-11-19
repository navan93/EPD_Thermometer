#include "timer.h"
#include "cpu.h"


#include "function_signatures.h"

// Global variable definitions (defined only once)
volatile uint16_t mTmrHi;

void T0_ISR(void) __interrupt (1)
{
	TCON &=~ 0x20;	//clear flag
	mTmrHi++;
}
