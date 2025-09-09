#include "Parameters.h"

#include "Memory.h"

KernelParams g_parameters = {};

void ParseKernelParams()
{
	// Default parameter values
	g_parameters.ASLR = true;
	g_parameters.InitProcess = "X:/Init";

	i8* p = g_bootInfo.Args;

	while (*p) {
		while (*p == ' ' || *p == '\t')
			++p;

		if (!*p)
			break;

		const i8* key = p;
		while (*p && *p != ' ' && *p != '\t' && *p != '=')
			++p;
		const usz keyLength = p - key;

		const i8* value = nullptr;

		if (*p == '=') {
			++p;

			if (*p != ' ' && *p != '\t') {
				value = p;
				while (*p && *p != ' ' && *p != '\t')
					++p;

				// Modifying this in place removes the need to copy all this to an external buffer,
				// making this potentially faster with more parameters present.
				// The irony of me writing this a few lines of code above of just linear string comparisons...
				*p = '\0';
				++p;
			}
		}

		if (!value && keyLength == 6 && MemoryCompare(key, "NoASLR", keyLength)) {
			g_parameters.ASLR = false;
		} else if (value && keyLength == 11 && MemoryCompare(key, "InitProcess", keyLength)) {
			g_parameters.InitProcess = value;
		}
	}
}
