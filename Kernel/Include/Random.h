#pragma once

#include "Core.h"

typedef struct Randomness {
	u32 Key[8];
	u32 Nonce[3];
	u32 Counter;
	u32 Buffer[16];
	usz Pos;
} Randomness;

void InitRandom(Randomness* generator, u32 key[8], u32 nonce[3], usz initialCounter);

void RandomU64(Randomness* generator, u64* output);

void RandomReseed(Randomness* generator, const u32* entropy, usz length);
void RandomReseedU64(Randomness* generator, u64 entropy);

u64 Random();
