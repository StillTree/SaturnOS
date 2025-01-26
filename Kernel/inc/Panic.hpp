#pragma once

#include "Core.hpp"

namespace SaturnKernel {

/// Hangs the kernel process, disabling interrupts and using the `hlt` assembly instruction.
[[noreturn]]
auto Hang() -> void;
/// A helper function. Use the `PANIC` macro instead.
[[noreturn]]
auto Panic(const i8* message, const i8* fileName, usize lineNumber) -> void;

}

/// Kernel panics, halting the whole system and printing the provided message.
#define SK_PANIC(message) SaturnKernel::Panic(message, __FILE__, __LINE__)
/// Asserts that the condition is true and panic if not.
#define SK_PANIC_ASSERT(condition, message)                                                                                                \
	do {                                                                                                                                   \
		if (!(condition)) {                                                                                                                \
			SaturnKernel::Panic("Assertion failure: " #condition "\n" message, __FILE__, __LINE__);                                        \
		}                                                                                                                                  \
	} while (false)
