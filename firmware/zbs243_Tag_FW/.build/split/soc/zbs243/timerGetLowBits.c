#include "timer.h"
#include "cpu.h"


#include "function_signatures.h"

uint8_t timerGetLowBits(void)
{
	return TL0;
}
