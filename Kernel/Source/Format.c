#include "Format.h"

// So apparently C now has constexpr, nice :D
static constexpr i8 HEX_DIGITS[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

usz NumberToHexStringLength(u64 number)
{
	usz length = 0;

	do {
		length++;
		number /= 16;
	} while (number > 0);

	return length;
}

void NumberToHexString(u64 number, i8* buffer, u8 zeroPad)
{
	usz i = NumberToHexStringLength(number);
	if (zeroPad < i) {
		zeroPad = i;
	}
	usz lengthDifference = zeroPad - i;

	for (usz j = 0; j < lengthDifference; j++) {
		buffer[j] = '0';
	}
	i = zeroPad;
	buffer[i] = '\0';

	while (i > lengthDifference) {
		buffer[--i] = HEX_DIGITS[number % 16];
		number /= 16;
	}
}

usz NumberToDecimalStringLength(u64 number)
{
	usz length = 0;

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

	usz i = NumberToDecimalStringLength(number);
	buffer[i] = '\0';

	while (number > 0 && i > 0) {
		buffer[--i] = (number % 10) + '0';
		number /= 10;
	}
}
