#pragma once

#include "Core.hpp"

namespace SaturnKernel {

auto TranslateScanCode(u8 scanCode) -> i8;
extern const i8 SET1_SCAN_CODES[];

}
