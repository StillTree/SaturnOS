#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	struct FramebufferLogger
	{
		void WriteChar(U8 character);
		void WriteString(const I8* string);
		void Clear();

		U32* framebuffer;
		U64  framebufferSize;
		U64  width;
		U64  height;
		U64  cursorPositionX;
		U64  cursorPositionY;
	};
}

