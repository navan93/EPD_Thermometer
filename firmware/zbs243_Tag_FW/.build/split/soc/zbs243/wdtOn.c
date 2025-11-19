#include "cpu.h"
#include "wdt.h"


#include "function_signatures.h"

void wdtOn()
{
	const uint8_t cfgPageBck = CFGPAGE;

	CFGPAGE = 4;
	WDTCONF |= 0x80;
	WDTENA = 1;

	CFGPAGE = cfgPageBck;
}
