#pragma once

#include "Core.h"

typedef struct FramebufferLogger {
	u32* Framebuffer;
	u64 FramebufferSize;
	u64 Width;
	u64 Height;
	u64 CursorPosX;
	u64 CursorPosY;
} FramebufferLogger;

void FramebufferWriteChar(FramebufferLogger* logger, u8 character);
void FramebufferWriteString(FramebufferLogger* logger, const i8* string);
void FramebufferShiftLine(FramebufferLogger* logger);
void FramebufferClear(FramebufferLogger* logger);
