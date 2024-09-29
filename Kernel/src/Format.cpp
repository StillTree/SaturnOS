#include "Format.hpp"

namespace SaturnKernel
{
	constexpr I8 HEX_DIGITS[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	USIZE NumberToHexStringLength(U64 number)
	{
		USIZE length = 3;

		do
		{
			length++;
			number /= 16;
		}
		while(number > 0);

		return length;
	}

	void NumberToHexString(U64 number, I8* buffer)
	{
		buffer[0] = '0';
		buffer[1] = 'x';

		if(number == 0)
		{
			buffer[2] = '0';
			buffer[3] = '\0';
			return;
		}

		USIZE i	  = NumberToHexStringLength(number) - 1;
		buffer[i] = '\0';

		while(number > 0 && i > 1)
		{
			buffer[--i] = HEX_DIGITS[number % 16];
			number	   /= 16;
		}
	}

	USIZE NumberToDecimalStringLength(U64 number)
	{
		USIZE length = 1;

		do
		{
			length++;
			number /= 10;
		}
		while(number > 0);

		return length;
	}

	void NumberToDecimalString(U64 number, I8* buffer)
	{
		if(number == 0)
		{
			buffer[0] = '0';
			buffer[1] = '\0';
			return;
		}

		USIZE i	  = NumberToDecimalStringLength(number) - 1;
		buffer[i] = '\0';

		while(number > 0 && i > 0)
		{
			buffer[--i] = (number % 10) + '0';
			number	   /= 10;
		}
	}
}
