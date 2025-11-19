#include "cpu.h"
#include "wdt.h"


#include "function_signatures.h"

void wdtOff()
{
	const uint8_t cfgPageBck = CFGPAGE;

	CFGPAGE = 4;
	WDTENA = 0;
	WDTCONF &=~ 0x80;

	CFGPAGE = cfgPageBck;
}
