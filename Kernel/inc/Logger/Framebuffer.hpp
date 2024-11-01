#pragma once

#include "Core.hpp"

namespace SaturnKernel {

struct FramebufferLogger {
	auto WriteChar(U8 character) -> void;
	auto WriteString(const I8* string) -> void;
	auto ShiftLine() -> void;
	auto Clear() -> void;

	U32* Framebuffer;
	U64 FramebufferSize;
	U64 Width;
	U64 Height;
	U64 CursorPositionX;
	U64 CursorPositionY;
};

}
