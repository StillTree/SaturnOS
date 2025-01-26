#pragma once

#include "Core.hpp"

namespace SaturnKernel {

struct FramebufferLogger {
	auto WriteChar(u8 character) -> void;
	auto WriteString(const i8* string) -> void;
	auto ShiftLine() -> void;
	auto Clear() -> void;

	u32* Framebuffer;
	u64 FramebufferSize;
	u64 Width;
	u64 Height;
	u64 CursorPositionX;
	u64 CursorPositionY;
};

}
