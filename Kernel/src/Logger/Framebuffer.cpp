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
			if(this->cursorPositionY + 40 >= this->height)
			{
				Clear();
				return;
			}

			this->cursorPositionY += 20;
			this->cursorPositionX  = 0;
			return;
		}

		if(character == L'\r')
		{
			this->cursorPositionX = 0;
			return;
		}

		if(character == L'\t')
		{
			if(this->cursorPositionX + 40 >= width)
			{
				WriteChar('\n');
				return;
			}

			this->cursorPositionX += 40;
			return;
		}

		if(this->cursorPositionX + 9 >= this->width)
		{
			WriteChar('\n');
		}

		for(USIZE y = 0; y < 20; y++)
		{
			for(USIZE x = 0; x < 10; x++)
			{
				U8 pixelIntensity					= g_fontBitmaps[charIndex][y][x];
				USIZE framebufferIndex				= (this->cursorPositionY + y) * this->width + (this->cursorPositionX + x);
				this->framebuffer[framebufferIndex] = (pixelIntensity << 16) | (pixelIntensity << 8) | pixelIntensity;
			}
		}

		this->cursorPositionX += 9;
	}

	void FramebufferLogger::WriteString(const I8* string)
	{
		USIZE i = 0;
		while(string[i])
		{
			WriteChar(string[i++]);
		}
	}

	void FramebufferLogger::Clear()
	{
		MemoryFill(this->framebuffer, 0, this->framebufferSize);

		this->cursorPositionY = 0;
		this->cursorPositionX = 0;
	}
}
