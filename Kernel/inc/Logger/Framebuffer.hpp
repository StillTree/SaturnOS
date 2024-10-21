#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	struct FramebufferLogger
	{
		void WriteChar(U8 character);
		void WriteString(const I8* string);
		void ShiftLine();
		void Clear();

		U32* Framebuffer;
		U64 FramebufferSize;
		U64 Width;
		U64 Height;
		U64 CursorPositionX;
		U64 CursorPositionY;
	};
}
