#include "Random.h"

#include "Instructions.h"
#include "Memory.h"

static RandomState g_random;

static u32 U32Rotate(u32 x, usz n) { return (x << n) | (x >> (32 - n)); }

static void ChaCha20QuarterRound(u32* a, u32* b, u32* c, u32* d)
{
	*a += *b;
	*d ^= *a;
	*d = U32Rotate(*d, 16);
	*c += *d;
	*b ^= *c;
	*b = U32Rotate(*b, 12);
	*a += *b;
	*d ^= *a;
	*d = U32Rotate(*d, 8);
	*c += *d;
	*b ^= *c;
	*b = U32Rotate(*b, 7);
}

static void ChaCha20CoreTransform(u32* output, const u32* input)
{
	u32 temp[16];
	MemoryCopy(input, temp, sizeof temp);

	for (usz i = 0; i < 10; ++i) {
		ChaCha20QuarterRound(&temp[0], &temp[4], &temp[8], &temp[12]);
		ChaCha20QuarterRound(&temp[1], &temp[5], &temp[9], &temp[13]);
		ChaCha20QuarterRound(&temp[2], &temp[6], &temp[10], &temp[14]);
		ChaCha20QuarterRound(&temp[3], &temp[7], &temp[11], &temp[15]);

		ChaCha20QuarterRound(&temp[0], &temp[5], &temp[10], &temp[15]);
		ChaCha20QuarterRound(&temp[1], &temp[6], &temp[11], &temp[12]);
		ChaCha20QuarterRound(&temp[2], &temp[7], &temp[8], &temp[13]);
		ChaCha20QuarterRound(&temp[3], &temp[4], &temp[9], &temp[14]);
	}

	for (usz i = 0; i < 16; ++i) {
		output[i] = temp[i] + input[i];
	}
}

static void ChaCha20Run(const u32* key, const u32* nonce, u32 counter, u32* outputBytes)
{
	u32 state[16];
	state[0] = 0x61707865U;
	state[1] = 0x3320646eU;
	state[2] = 0x79622d32U;
	state[3] = 0x6b206574U;

	for (usz i = 4; i < 12; ++i) {
		state[i] = key[i - 4];
	}

	state[12] = counter;
	state[13] = nonce[0];
	state[14] = nonce[1];
	state[15] = nonce[2];

	u32 outputState[16];
	ChaCha20CoreTransform(outputState, state);

	for (usz i = 0; i < 16; ++i) {
		outputBytes[i] = outputState[i];
	}
}

void InitRandomness()
{
	u64 tsc = ReadTSC();

	u32 key[8];
	key[0] = tsc & 0xffffffff;
	key[1] = tsc >> 32;
	key[2] = g_bootInfo.ContextSwitchFunctionPage & 0xffffffff;
	key[3] = g_bootInfo.ContextSwitchFunctionPage >> 32;
	key[4] = 123;
	key[5] = 123;
	key[6] = 123;
	key[7] = 123;

	u32 nonce[3];
	nonce[0] = g_bootInfo.XSDTPhysAddr & 0xffffffff;
	nonce[1] = (g_bootInfo.XSDTPhysAddr >> 32) ^ (g_bootInfo.ContextSwitchFunctionPage & 0xffffffff);
	nonce[2] = g_bootInfo.ContextSwitchFunctionPage >> 32;

	MemoryCopy(key, g_random.Key, sizeof g_random.Key);
	MemoryCopy(nonce, g_random.Nonce, sizeof g_random.Nonce);
	g_random.Counter = 0;
	g_random.BufferPos = sizeof g_random.KeystreamBuffer;
	MemoryFill(g_random.KeystreamBuffer, 0, sizeof g_random.KeystreamBuffer);
}

static void RandomnessRefill(RandomState* generator)
{
	ChaCha20Run(generator->Key, generator->Nonce, generator->Counter, (u32*)generator->KeystreamBuffer);
	++generator->Counter;
	generator->BufferPos = 0;
}

void RandomnessReseed(RandomState* generator, const u32* entropy, usz length)
{
	for (usz i = 0; i < length; ++i) {
		generator->Key[i % 8] ^= entropy[i];
	}

	u32 tempKeystream[16];
	++generator->Counter;
	ChaCha20Run(generator->Key, generator->Nonce, generator->Counter, tempKeystream);

	MemoryCopy(tempKeystream, generator->Key, sizeof generator->Key);
	MemoryCopy(tempKeystream + 8, generator->Nonce, sizeof generator->Nonce);

	MemoryFill(tempKeystream, 0, sizeof tempKeystream);

	generator->BufferPos = sizeof generator->KeystreamBuffer;
}

void RandomBytes(RandomState* generator, void* output, usz length)
{
	u8* p = (u8*)output;

	for (usz i = 0; i < length; ++i) {
		if (generator->BufferPos >= sizeof generator->KeystreamBuffer) {
			RandomnessRefill(generator);
		}

		p[i] = generator->KeystreamBuffer[generator->BufferPos];
		++generator->BufferPos;
	}
}

u64 RandomU64()
{
	u64 result = 0;
	RandomBytes(&g_random, &result, 8);

	return result;
}

u32 RandomU32()
{
	u32 result = 0;
	RandomBytes(&g_random, &result, 4);

	return result;
}

u16 RandomU16()
{
	u16 result = 0;
	RandomBytes(&g_random, &result, 2);

	return result;
}

u8 RandomU8()
{
	u8 result = 0;
	RandomBytes(&g_random, &result, 1);

	return result;
}

i64 RandomI64()
{
	i64 result = 0;
	RandomBytes(&g_random, &result, 8);

	return result;
}

i32 RandomI32()
{
	i32 result = 0;
	RandomBytes(&g_random, &result, 4);

	return result;
}

i16 RandomI16()
{
	i16 result = 0;
	RandomBytes(&g_random, &result, 2);

	return result;
}

i8 RandomI8()
{
	i8 result = 0;
	RandomBytes(&g_random, &result, 1);

	return result;
}

u64 Random()
{
	usz rand = 1;

	// if (!g_cpuInformation.SupportsRDRAND) {
	//	return RandomRDRAND();
	// }

	// TODO: Actually implement a pseudo-random algorithm
	return rand++;
}
