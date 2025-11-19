#include "cpu.h"
#include "wdt.h"


#include "function_signatures.h"

void wdtPet()
{
	const uint8_t cfgPageBck = CFGPAGE;

	CFGPAGE = 4;
	WDTPET = 0;

	CFGPAGE = cfgPageBck;
}
