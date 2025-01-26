#include "Format.h"

// So apparently C now has constexpr, nice :D
static constexpr i8 HEX_DIGITS[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

usz NumberToHexStringLength(u64 number)
{
	usz length = 3;

	do {
		length++;
		number /= 16;
	} while (number > 0);

	return length;
}

void NumberToHexString(u64 number, i8* buffer)
{
	buffer[0] = '0';
	buffer[1] = 'x';

	if (number == 0) {
		buffer[2] = '0';
		buffer[3] = '\0';
		return;
	}

	usz i = NumberToHexStringLength(number) - 1;
	buffer[i] = '\0';

	while (number > 0 && i > 1) {
		buffer[--i] = HEX_DIGITS[number % 16];
		number /= 16;
	}
}

usz NumberToDecimalStringLength(u64 number)
{
	usz length = 1;

	do {
		length++;
		number /= 10;
	} while (number > 0);

	return length;
}

void NumberToDecimalString(u64 number, i8* buffer)
{
	if (number == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}

	usz i = NumberToDecimalStringLength(number) - 1;
	buffer[i] = '\0';

	while (number > 0 && i > 0) {
		buffer[--i] = (number % 10) + '0';
		number /= 10;
	}
}
