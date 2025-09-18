#include "Random.h"

#include "CPUInfo.h"
#include "GDT.h"
#include "Instructions.h"
#include "Memory.h"

static RandomState g_random;

static u32 U32Rotate(u32 x, u32 n) { return (x << n) | (x >> (32 - n)); }

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

	MemoryFill(temp, 0, sizeof temp);
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

	ChaCha20CoreTransform(outputBytes, state);

	MemoryFill(state, 0, sizeof state);
}

void RandomnessInit()
{
	g_random.Key[0] = (u32)ReadTSC();
	g_random.Key[1] = (u32)(ReadTSC() >> 32);
	g_random.Key[2] = (u32)g_bootInfo.ContextSwitchFunctionPage;
	g_random.Key[3] = (u32)(g_bootInfo.ContextSwitchFunctionPage >> 32);

	if (g_cpuInformation.SupportsRDSEED) {
		u64 rdseed;
		// Key
		if (RDSEED(&rdseed)) {
			g_random.Key[4] = (u32)rdseed;
			g_random.Key[5] = (u32)(rdseed >> 32);
		} else {
			u64 tsc = ReadTSC();
			__asm__ volatile("pause");
			tsc ^= ReadTSC();
			g_random.Key[4] = (u32)tsc;
			g_random.Key[5] = (u32)(tsc >> 32);
		}

		// Nonce
		if (RDSEED(&rdseed)) {
			g_random.Nonce[1] = (u32)rdseed;
			g_random.Nonce[2] = (u32)(rdseed >> 32);
		} else {
			__asm__ volatile("pause");
			u64 tsc = ReadTSC();
			g_random.Nonce[1] = (u32)tsc;
			g_random.Nonce[2] = (u32)(tsc >> 32);
		}
	} else {
		// Key
		u64 tsc = ReadTSC();
		__asm__ volatile("pause");
		tsc ^= ReadTSC();
		g_random.Key[4] = (u32)tsc;
		g_random.Key[5] = (u32)(tsc >> 32);

		// Nonce
		__asm__ volatile("pause");
		tsc = ReadTSC();
		g_random.Nonce[1] = (u32)tsc;
		g_random.Nonce[2] = (u32)(tsc >> 32);
	}

	g_random.Key[6] = (u32)(g_bootInfo.MemoryMapEntries ^ (u64)&g_kernelInterruptStack);
	g_random.Key[7] = (u32)(ReadTSC() ^ (u64)&g_random ^ (u64)&ReadTSC);

	g_random.Nonce[0] = (u32)g_bootInfo.XSDTPhysAddr;
	g_random.Nonce[1] ^= (u32)((g_bootInfo.XSDTPhysAddr >> 32) ^ (g_bootInfo.PhysicalMemoryOffset & 0xffffffff));
	g_random.Nonce[2] ^= g_bootInfo.PhysicalMemoryOffset >> 32;

	// TODO: Rekey when about to overrun
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

void RandomnessReseed(const u32* entropy, usz length)
{
	for (usz i = 0; i < length; ++i) {
		g_random.Key[i % 8] ^= entropy[i];
	}

	u32 tempKeystream[16];
	++g_random.Counter;
	ChaCha20Run(g_random.Key, g_random.Nonce, g_random.Counter, tempKeystream);

	MemoryCopy(tempKeystream, g_random.Key, sizeof g_random.Key);
	MemoryCopy(tempKeystream + 8, g_random.Nonce, sizeof g_random.Nonce);

	MemoryFill(tempKeystream, 0, sizeof tempKeystream);

	g_random.BufferPos = sizeof g_random.KeystreamBuffer;
}

void RandomBytes(void* output, usz length)
{
	u8* p = (u8*)output;

	for (usz i = 0; i < length; ++i) {
		if (g_random.BufferPos >= sizeof g_random.KeystreamBuffer) {
			RandomnessRefill(&g_random);
		}

		p[i] = g_random.KeystreamBuffer[g_random.BufferPos];
		++g_random.BufferPos;
	}
}

u64 RandomU64()
{
	u64 result = 0;
	RandomBytes(&result, 8);

	return result;
}

u32 RandomU32()
{
	u32 result = 0;
	RandomBytes(&result, 4);

	return result;
}

u16 RandomU16()
{
	u16 result = 0;
	RandomBytes(&result, 2);

	return result;
}

u8 RandomU8()
{
	u8 result = 0;
	RandomBytes(&result, 1);

	return result;
}

i64 RandomI64()
{
	i64 result = 0;
	RandomBytes(&result, 8);

	return result;
}

i32 RandomI32()
{
	i32 result = 0;
	RandomBytes(&result, 4);

	return result;
}

i16 RandomI16()
{
	i16 result = 0;
	RandomBytes(&result, 2);

	return result;
}

i8 RandomI8()
{
	i8 result = 0;
	RandomBytes(&result, 1);

	return result;
}
