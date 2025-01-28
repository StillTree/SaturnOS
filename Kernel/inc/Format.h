#pragma once

#include "Core.h"

usz NumberToHexStringLength(u64 number);
void NumberToHexString(u64 number, i8* buffer);

usz NumberToDecimalStringLength(u64 number);
void NumberToDecimalString(u64 number, i8* buffer);

#define MAX_HEX_LENGTH 18
#define MAX_DECIMAL_LENGTH 20
