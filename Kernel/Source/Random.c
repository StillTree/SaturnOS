#include "Random.h"

#include "CPUInfo.h"

u64 Random()
{
	usz rand = 1;

	// if (!g_cpuInformation.SupportsRDRAND) {
		return RandomRDRAND();
	// }

	// TODO: Actually implement a pseudo-random algorithm
	// return rand++;
}
