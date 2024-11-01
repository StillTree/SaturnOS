#pragma once

#include "Core.hpp"

namespace SaturnKernel {

auto NumberToHexStringLength(U64 number) -> USIZE;
void NumberToHexString(U64 number, I8* buffer);

auto NumberToDecimalStringLength(U64 number) -> USIZE;
void NumberToDecimalString(U64 number, I8* buffer);

constexpr USIZE MAX_HEX_LENGTH = 16;
constexpr USIZE MAX_DECIMAL_LENGTH = 20;

}
