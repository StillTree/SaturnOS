#include "Logger/Framebuffer.hpp"

#include "Memory.hpp"

namespace SaturnKernel
{
	extern const U8 g_fontBitmaps[96][20][10];

	void FramebufferLogger::WriteChar(U8 character)
	{
		USIZE charIndex = character > 126 ? 95 : character - 32;

		if(character == L'\n')
		{
			if(cursorPositionY + 40 >= height)
			{
				ShiftLine();
				return;
			}

			cursorPositionY += 20;
			cursorPositionX  = 0;
			return;
		}

		if(character == L'\r')
		{
			cursorPositionX = 0;
			return;
		}

		if(character == L'\t')
		{
			if(cursorPositionX + 40 >= width)
			{
				WriteChar('\n');
				return;
			}

			cursorPositionX += 40;
			return;
		}

		if(cursorPositionX + 9 >= width)
		{
			WriteChar('\n');
		}

		for(USIZE y = 0; y < 20; y++)
		{
			for(USIZE x = 0; x < 10; x++)
			{
				U8 pixelIntensity					= g_fontBitmaps[charIndex][y][x];
				USIZE framebufferIndex				= (cursorPositionY + y) * width + (cursorPositionX + x);
				framebuffer[framebufferIndex] = (pixelIntensity << 16) | (pixelIntensity << 8) | pixelIntensity;
			}
		}

		cursorPositionX += 9;
	}

	void FramebufferLogger::WriteString(const I8* string)
	{
		USIZE i = 0;
		while(string[i])
		{
			WriteChar(string[i++]);
		}
	}

	void FramebufferLogger::ShiftLine()
	{
		MemoryCopy(framebuffer + 20 * width, framebuffer, cursorPositionY * width * 4);
		cursorPositionX = 0;
		MemoryFill(framebuffer + cursorPositionY * width, 0, 20 * width * 4);
	}

	void FramebufferLogger::Clear()
	{
		MemoryFill(framebuffer, 0, framebufferSize);

		cursorPositionY = 0;
		cursorPositionX = 0;
	}
}
