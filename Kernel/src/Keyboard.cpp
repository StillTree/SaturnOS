#include "Keyboard.hpp"

namespace SaturnKernel {

constexpr i8 const SET1_SCAN_CODES[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '?', '\t', 'q', 'w', 'e', 'r', 't',
	'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '?', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c',
	'v', 'b', 'n', 'm', ',', '.', '/', '?', '*', '?', ' ' };

auto TranslateScanCode(u8 scanCode) -> i8
{
	if (scanCode < 2 || scanCode >= 58)
		return '?';

	return SET1_SCAN_CODES[scanCode - 2];
}

}
