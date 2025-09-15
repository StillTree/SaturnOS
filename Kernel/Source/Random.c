#include "Random.h"

#include "CPUInfo.h"
#include "Memory.h"

extern u64 RandomRDRAND();

static u64 g_xorShiftState = 0x1234567890abcdefULL;

static u32 U32Rotate(u32 x, usz n) { return (x << n) | (x >> (32 - n)); }

static void QuarterRound(u32* a, u32* b, u32* c, u32* d)
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

static void ChaCha20Block(u32 output[16], const u32 input[16])
{
	u32 temp[16];
	MemoryCopy(input, temp, sizeof(temp));

	for (usz i = 0; i < 10; ++i) {
		QuarterRound(&temp[0], &temp[4], &temp[8], &temp[12]);
		QuarterRound(&temp[1], &temp[5], &temp[9], &temp[13]);
		QuarterRound(&temp[2], &temp[6], &temp[10], &temp[14]);
		QuarterRound(&temp[3], &temp[7], &temp[11], &temp[15]);

		QuarterRound(&temp[0], &temp[5], &temp[10], &temp[15]);
		QuarterRound(&temp[1], &temp[6], &temp[11], &temp[12]);
		QuarterRound(&temp[2], &temp[7], &temp[8], &temp[13]);
		QuarterRound(&temp[3], &temp[4], &temp[9], &temp[14]);
	}

	for (usz i = 0; i < 16; ++i) {
		output[i] = temp[i] + input[i];
	}
}

static void ChaCha20KeystreamBlockBytes(u32 outputBytes[16], const u32 key[8], const u32 nonce[3], u32 counter)
{
	u32 state[16];
	state[0] = 0x61707865U;
	state[1] = 0x3320646eU;
	state[2] = 0x79622d32U;
	state[3] = 0x6b206574U;

	for (usz i = 0; i < 8; ++i) {
		state[4 + i] = key[i];
	}

	state[12] = counter;
	state[13] = nonce[0];
	state[14] = nonce[1];
	state[15] = nonce[2];

	u32 outputState[16];
	ChaCha20Block(outputState, state);

	for (usz i = 0; i < 16; ++i) {
		outputBytes[i] = outputState[i];
	}
}

void InitRandom(ChaState* generator, u32 key[8], u32 nonce[3], usz initialCounter)
{
	MemoryCopy(key, generator->Key, sizeof generator->Key);
	MemoryCopy(nonce, generator->Nonce, sizeof generator->Nonce);
	generator->Counter = initialCounter;
	generator->Pos = 16;
	MemoryFill(generator->Buffer, 0, sizeof generator->Buffer);
}

static void ChaRefill(ChaState* generator)
{
	ChaCha20KeystreamBlockBytes(generator->Buffer, generator->Key, generator->Nonce, generator->Counter);
	++generator->Counter;
	generator->Pos = 0;
}

void RandomU64(ChaState* generator, u64* output)
{
	if (generator->Pos >= 15) {
		ChaRefill(generator);
	}

	*output = generator->Buffer[generator->Pos] | ((u64)generator->Buffer[generator->Pos + 1] << 32);
	generator->Pos += 2;
}

void RandomReseedU64(ChaState* generator, u64 entropy)
{
	u32 nice[2];
	nice[0] = entropy & 0xffffffff;
	nice[1] = (entropy >> 32) & 0xffffffff;

	RandomReseed(generator, nice, 2);
}

void RandomReseed(ChaState* generator, const u32* entropy, usz length)
{
	for (usz i = 0; i < length; ++i) {
		generator->Key[i % 8] ^= entropy[i];
	}

    u32 ks[16];
    ChaCha20KeystreamBlockBytes(ks, generator->Key, generator->Nonce, generator->Counter);
    ++generator->Counter;
    MemoryCopy(ks, generator->Key, 32);
    MemoryFill(ks, 0, sizeof ks);
    generator->Pos = 16;
}

static u64 RandomXorShift64()
{
	u64 x = g_xorShiftState;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	g_xorShiftState = x;

	return x;
}

u64 Random()
{
	if (g_cpuInformation.SupportsRDRAND) {
		return RandomRDRAND();
	}

	return RandomXorShift64();
}
