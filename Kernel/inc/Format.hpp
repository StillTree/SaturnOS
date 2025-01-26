#pragma once

#include "Core.hpp"

namespace SaturnKernel {

auto NumberToHexStringLength(u64 number) -> usize;
void NumberToHexString(u64 number, i8* buffer);

auto NumberToDecimalStringLength(u64 number) -> usize;
void NumberToDecimalString(u64 number, i8* buffer);

constexpr usize MAX_HEX_LENGTH = 16;
constexpr usize MAX_DECIMAL_LENGTH = 20;

}
