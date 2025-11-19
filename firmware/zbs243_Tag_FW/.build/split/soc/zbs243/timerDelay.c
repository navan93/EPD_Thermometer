#include "timer.h"
#include "cpu.h"


#include "function_signatures.h"

void timerDelay(uint32_t ticks)
{
	uint32_t start = timerGet();
	while (timerGet() - start <= ticks);
}
