#include "Keyboard.h"

constexpr static i8 SET1_SCAN_CODES[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '?', '\t', 'q', 'w', 'e', 'r', 't',
	'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '?', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c',
	'v', 'b', 'n', 'm', ',', '.', '/', '?', '*', '?', ' ' };

i8 TranslateScanCode(u8 scanCode)
{
	if (scanCode < 2 || scanCode >= 58)
		return '?';

	return SET1_SCAN_CODES[scanCode - 2];
}
