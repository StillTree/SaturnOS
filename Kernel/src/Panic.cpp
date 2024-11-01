#include "Panic.hpp"

#include "Format.hpp"
#include "InOut.hpp"
#include "Memory.hpp"

namespace SaturnKernel {

extern const U8 FONT_BITMAPS[96][20][10];

auto Hang() -> void
{
	while (true)
		__asm__ volatile("cli; hlt");
}

static auto PanicFramebufferWriteChar(U8 character, U32* framebuffer, USIZE& positionX, USIZE& positionY) -> void
{
	USIZE charIndex = character > 126 ? 95 : character - 32;

	if (character == L'\n') {
		if (positionY + 40 >= g_bootInfo.FramebufferHeight) {
			MemoryFill(g_bootInfo.MemoryMap, 0, g_bootInfo.FramebufferSize);
			return;
		}

		positionY += 20;
		positionX = 0;
		return;
	}

	if (character == L'\r') {
		positionX = 0;
		return;
	}

	if (positionX + 9 >= g_bootInfo.FramebufferWidth) {
		PanicFramebufferWriteChar('\n', framebuffer, positionX, positionY);
	}

	for (USIZE y = 0; y < 20; y++) {
		for (USIZE x = 0; x < 10; x++) {
			U8 pixelIntensity = FONT_BITMAPS[charIndex][y][x];
			USIZE framebufferIndex = ((positionY + y) * g_bootInfo.FramebufferWidth) + (positionX + x);
			framebuffer[framebufferIndex] = (pixelIntensity << 16) | (pixelIntensity << 8) | pixelIntensity;
		}
	}

	positionX += 9;
}

static auto PanicReinitializeSerialConsole() -> void
{
	OutputU8(0x3f8 + 1, 0x00);
	OutputU8(0x3f8 + 3, 0x80);
	OutputU8(0x3f8, 0x03);
	OutputU8(0x3f8 + 1, 0x00);
	OutputU8(0x3f8 + 3, 0x03);
	OutputU8(0x3f8 + 2, 0xc7);
	OutputU8(0x3f8 + 4, 0x0b);
	// Set in loopback mode
	OutputU8(0x3f8 + 4, 0x1e);

	OutputU8(0x3f8, 0xae);

	// If we didn't get back the exact same byte that we sent in loopback mode,
	// the device is not functioning corretly and should not be used
	if (InputU8(0x3f8 + 0) != 0xae) {
		return;
	}

	// If it is functioning correctly we set it in normal operation mode
	OutputU8(0x3f8 + 4, 0x0f);
}

static auto PanicSerialWriteChar(U8 character) -> void
{
	if (character > 126) {
		character = '?';
	}

	OutputU8(0x3f8, character);
}

static auto PanicWriteString(const I8* string, U32* framebuffer, USIZE& positionX, USIZE& positionY) -> void
{
	USIZE i = 0;
	while (string[i]) {
		PanicFramebufferWriteChar(string[i], framebuffer, positionX, positionY);
		PanicSerialWriteChar(string[i]);
		i++;
	}
}

static auto PanicWriteChar(I8 character, U32* framebuffer, USIZE& positionX, USIZE& positionY) -> void
{
	PanicSerialWriteChar(character);
	PanicFramebufferWriteChar(character, framebuffer, positionX, positionY);
}

auto Panic(const I8* message, const I8* fileName, USIZE lineNumber) -> void
{
	USIZE cursorPositionX = 0;
	USIZE cursorPositionY = 0;
	// The bootloader will refuse to boot the system if there is no viable framebuffer to use, so a framebuffer is guaranteed to be
	// present.
	U32* framebuffer = g_bootInfo.Framebuffer;

	MemoryFill(framebuffer, 0, g_bootInfo.FramebufferSize);
	// We don't know if the COM1 serial output device has been initialized in any way, so we initialize it here ourselves
	PanicReinitializeSerialConsole();

	PanicWriteString("!!!UNRECOVERABLE KERNEL PANIC!!!\n", framebuffer, cursorPositionX, cursorPositionY);

	// Print out the file name and number on which the panic occured
	PanicWriteString("Occured at: ", framebuffer, cursorPositionX, cursorPositionY);
	PanicWriteString(fileName, framebuffer, cursorPositionX, cursorPositionY);
	PanicWriteChar(':', framebuffer, cursorPositionX, cursorPositionY);
	I8 lineNumberString[MAX_DECIMAL_LENGTH];
	NumberToDecimalString(lineNumber, lineNumberString);
	PanicWriteString(lineNumberString, framebuffer, cursorPositionX, cursorPositionY);
	PanicWriteChar('\n', framebuffer, cursorPositionX, cursorPositionY);

	PanicWriteString(message, framebuffer, cursorPositionX, cursorPositionY);
	PanicWriteString("\n\n", framebuffer, cursorPositionX, cursorPositionY);
	PanicWriteString("Please restart your computer to continue.\n", framebuffer, cursorPositionX, cursorPositionY);
	PanicWriteString("Hanging...\n", framebuffer, cursorPositionX, cursorPositionY);

	Hang();
}

}
