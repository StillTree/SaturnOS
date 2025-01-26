#pragma once

#include "Core.hpp"

namespace SaturnKernel {

auto OutputU8(u16 port, u8 value) -> void;
auto InputU8(u16 port) -> u8;
auto IOWait() -> void;

}
