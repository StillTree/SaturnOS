#pragma once

#include "Core.h"

/// Hangs the kernel process, disabling interrupts and using the `hlt` assembly instruction.
[[noreturn]]
void Hang();
/// A helper function. Use the `PANIC` macro instead.
[[noreturn]]
void Panic(const i8* message, const i8* fileName, usz lineNumber);

/// Kernel panics, halting the whole system and printing the provided message.
#define SK_PANIC(message) Panic(message, __FILE__, __LINE__)
/// Asserts that the condition is true and panic if not.
#define SK_PANIC_ASSERT(condition, message)                                                                                                \
	do {                                                                                                                                   \
		if (!(condition)) {                                                                                                                \
			Panic("Assertion failure: " #condition "\n" message, __FILE__, __LINE__);                                                      \
		}                                                                                                                                  \
	} while (false)
/// Panics if the function's result value is not `ResultOk`.
#define SK_PANIC_ON_ERROR(function, message)                                                                                               \
	do {                                                                                                                                   \
		if ((function) != ResultOk) {                                                                                                      \
			Panic("Function " #function " failed.\n" message, __FILE__, __LINE__);                                                         \
		}                                                                                                                                  \
	} while (false)
