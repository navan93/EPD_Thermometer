#include "cpu.h"
#include "wdt.h"


#include "function_signatures.h"

void wdtDeviceReset()
{
	CFGPAGE = 4;
	WDTCONF = 0x80;
	WDTENA = 1;
	WDTRSTVALH = 0xff;
	WDTRSTVALM = 0xff;
	WDTRSTVALL = 0xff;
	while(1);
}
