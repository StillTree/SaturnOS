#include "Keyboard.hpp"

namespace SaturnKernel
{
	I8 const g_set1ScanCodes[] = { '1',	 '2', '3', '4',	 '5', '6', '7', '8',  '9', '0', '-', '=', '?', '\t', 'q', 'w', 'e', 'r', 't',
								   'y',	 'u', 'i', 'o',	 'p', '[', ']', '\n', '?', 'a', 's', 'd', 'f', 'g',	 'h', 'j', 'k', 'l', ';',
								   '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',  'b', 'n', 'm', ',', '.', '/',	 '?', '*', '?', ' ' };

	I8 TranslateScanCode(U8 scanCode)
	{
		if(scanCode < 2 || scanCode >= 58)
			return '?';

		return g_set1ScanCodes[scanCode - 2];
	}
}
