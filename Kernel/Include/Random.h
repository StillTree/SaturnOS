#pragma once

#include "Core.h"

typedef struct RandomState {
	u32 Key[8];
	u32 Nonce[3];
	u32 Counter;

	u8 KeystreamBuffer[64];
	usz BufferPos;
} RandomState;

void InitRandomness();

void RandomnessReseed(const u32* entropy, usz length);

void RandomBytes(void* output, usz length);
u64 RandomU64();
u32 RandomU32();
u16 RandomU16();
u8 RandomU8();
i64 RandomI64();
i32 RandomI32();
i16 RandomI16();
i8 RandomI8();

u64 Random();
