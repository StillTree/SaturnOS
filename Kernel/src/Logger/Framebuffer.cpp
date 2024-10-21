#include "Logger/Framebuffer.hpp"

#include "Memory.hpp"

namespace SaturnKernel
{
	extern const U8 FONT_BITMAPS[96][20][10];

	void FramebufferLogger::WriteChar(U8 character)
	{
		USIZE charIndex = character > 126 ? 95 : character - 32;

		if(character == L'\n')
		{
			if(CursorPositionY + 40 >= Height)
			{
				ShiftLine();
				return;
			}

			CursorPositionY += 20;
			CursorPositionX	 = 0;
			return;
		}

		if(character == L'\r')
		{
			CursorPositionX = 0;
			return;
		}

		if(character == L'\t')
		{
			if(CursorPositionX + 40 >= Width)
			{
				WriteChar('\n');
				return;
			}

			CursorPositionX += 40;
			return;
		}

		if(CursorPositionX + 9 >= Width)
		{
			WriteChar('\n');
		}

		for(USIZE y = 0; y < 20; y++)
		{
			for(USIZE x = 0; x < 10; x++)
			{
				U8 pixelIntensity			  = FONT_BITMAPS[charIndex][y][x];
				USIZE framebufferIndex		  = ((CursorPositionY + y) * Width) + (CursorPositionX + x);
				Framebuffer[framebufferIndex] = (pixelIntensity << 16) | (pixelIntensity << 8) | pixelIntensity;
			}
		}

		CursorPositionX += 9;
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
		MemoryCopy(Framebuffer + (20 * Width), Framebuffer, CursorPositionY * Width * 4);
		CursorPositionX = 0;
		MemoryFill(Framebuffer + (CursorPositionY * Width), 0, 20 * Width * 4);
	}

	void FramebufferLogger::Clear()
	{
		MemoryFill(Framebuffer, 0, FramebufferSize);

		CursorPositionY = 0;
		CursorPositionX = 0;
	}
}
