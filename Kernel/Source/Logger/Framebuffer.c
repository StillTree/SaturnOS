#include "Logger/Framebuffer.h"

#include "Memory.h"

extern const u8 FONT_BITMAPS[96][20][10];

void FramebufferWriteChar(FramebufferLogger* logger, u8 character)
{
	usz charIndex = character > 126 ? 95 : character - 32;

	if (character == L'\n') {
		if (logger->CursorPosY + 40 >= logger->Height) {
			FramebufferShiftLine(logger);
			return;
		}

		logger->CursorPosY += 20;
		logger->CursorPosX = 0;
		return;
	}

	if (character == L'\r') {
		logger->CursorPosX = 0;
		return;
	}

	if (character == L'\t') {
		if (logger->CursorPosX + 40 >= logger->Width) {
			FramebufferWriteChar(logger, '\n');
			return;
		}

		logger->CursorPosX += 40;
		return;
	}

	if (logger->CursorPosX + 9 >= logger->Width) {
		FramebufferWriteChar(logger, '\n');
	}

	for (usz y = 0; y < 20; y++) {
		for (usz x = 0; x < 10; x++) {
			u8 pixelIntensity = FONT_BITMAPS[charIndex][y][x];
			usz framebufferIndex = ((logger->CursorPosY + y) * logger->Width) + (logger->CursorPosX + x);
			logger->Framebuffer[framebufferIndex] = (pixelIntensity << 16) | (pixelIntensity << 8) | pixelIntensity;
		}
	}

	logger->CursorPosX += 9;
}

void FramebufferWriteString(FramebufferLogger* logger, const i8* string)
{
	usz i = 0;
	while (string[i]) {
		FramebufferWriteChar(logger, string[i++]);
	}
}

void FramebufferShiftLine(FramebufferLogger* logger)
{
	MemoryCopy(logger->Framebuffer + (20 * logger->Width), logger->Framebuffer, logger->CursorPosY * logger->Width * 4);
	logger->CursorPosX = 0;
	MemoryFill(logger->Framebuffer + (logger->CursorPosY * logger->Width), 0, 20 * logger->Width * 4);
}

void FramebufferClear(FramebufferLogger* logger)
{
	MemoryFill(logger->Framebuffer, 0, logger->FramebufferSize);

	logger->CursorPosY = 0;
	logger->CursorPosX = 0;
}
