#include "Format.hpp"

namespace SaturnKernel {

constexpr i8 HEX_DIGITS[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

auto NumberToHexStringLength(u64 number) -> usize
{
	usize length = 3;

	do {
		length++;
		number /= 16;
	} while (number > 0);

	return length;
}

auto NumberToHexString(u64 number, i8* buffer) -> void
{
	buffer[0] = '0';
	buffer[1] = 'x';

	if (number == 0) {
		buffer[2] = '0';
		buffer[3] = '\0';
		return;
	}

	usize i = NumberToHexStringLength(number) - 1;
	buffer[i] = '\0';

	while (number > 0 && i > 1) {
		buffer[--i] = HEX_DIGITS[number % 16];
		number /= 16;
	}
}

auto NumberToDecimalStringLength(u64 number) -> usize
{
	usize length = 1;

	do {
		length++;
		number /= 10;
	} while (number > 0);

	return length;
}

auto NumberToDecimalString(u64 number, i8* buffer) -> void
{
	if (number == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}

	usize i = NumberToDecimalStringLength(number) - 1;
	buffer[i] = '\0';

	while (number > 0 && i > 0) {
		buffer[--i] = (number % 10) + '0';
		number /= 10;
	}
}

}
