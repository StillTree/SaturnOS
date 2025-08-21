#pragma once

// Yes, yes, I am aware I could have used the stdint.h header,
// but SaturnOS only runs on x86, 64-bit, so it doesn't matter if I use these or the variable sized builtins
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

typedef u64 usz;

constexpr i8 I8_MAX = 0x7f;
constexpr i16 I16_MAX = 0x7fff;
constexpr i32 I32_MAX = 0x7fffffff;
constexpr i64 I64_MAX = 0x7fffffffffffffff;

constexpr u8 U8_MAX = 0xff;
constexpr u16 U16_MAX = 0xffff;
constexpr u32 U32_MAX = 0xffffffff;
constexpr u64 U64_MAX = 0xffffffffffffffff;

constexpr usz USZ_MAX = U64_MAX;
