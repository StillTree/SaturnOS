#include "Panic.h"

#include "Format.h"
#include "InOut.h"
#include "Memory.h"

extern const u8 FONT_BITMAPS[96][20][10];

void Hang()
{
	while (true)
		__asm__ volatile("cli; hlt");
}

void PanicFramebufferWriteChar(u8 character, u32* framebuffer, usz* positionX, usz* positionY)
{
	usz charIndex = character > 126 ? 95 : character - 32;

	if (character == L'\n') {
		if (*positionY + 40 >= g_bootInfo.FramebufferHeight) {
			MemoryFill(g_bootInfo.MemoryMap, 0, g_bootInfo.FramebufferSize);
			return;
		}

		*positionY += 20;
		*positionX = 0;
		return;
	}

	if (character == L'\r') {
		*positionX = 0;
		return;
	}

	if (*positionX + 9 >= g_bootInfo.FramebufferWidth) {
		PanicFramebufferWriteChar('\n', framebuffer, positionX, positionY);
	}

	for (usz y = 0; y < 20; y++) {
		for (usz x = 0; x < 10; x++) {
			u8 pixelIntensity = FONT_BITMAPS[charIndex][y][x];
			usz framebufferIndex = ((*positionY + y) * g_bootInfo.FramebufferWidth) + (*positionX + x);
			framebuffer[framebufferIndex] = (pixelIntensity << 16) | (pixelIntensity << 8) | pixelIntensity;
		}
	}

	*positionX += 9;
}

void PanicReinitializeSerialConsole()
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

void PanicSerialWriteChar(u8 character)
{
	if (character > 126) {
		character = '?';
	}

	OutputU8(0x3f8, character);
}

void PanicWriteString(const i8* string, u32* framebuffer, usz* positionX, usz* positionY)
{
	usz i = 0;
	while (string[i]) {
		PanicFramebufferWriteChar(string[i], framebuffer, positionX, positionY);
		PanicSerialWriteChar(string[i]);
		i++;
	}
}

void PanicWriteChar(i8 character, u32* framebuffer, usz* positionX, usz* positionY)
{
	PanicSerialWriteChar(character);
	PanicFramebufferWriteChar(character, framebuffer, positionX, positionY);
}

void PanicClearScreen(u32* framebuffer)
{
	for (usz y = 0; y < g_bootInfo.FramebufferHeight; y++) {
		for (usz x = 0; x < g_bootInfo.FramebufferWidth; x++) {
			usz framebufferIndex = (y * g_bootInfo.FramebufferWidth) + x;
			framebuffer[framebufferIndex] = 0;
		}
	}
}

void Panic(const i8* message, const i8* fileName, usz lineNumber)
{
	usz cursorPositionX = 0;
	usz cursorPositionY = 0;
	// The bootloader will refuse to boot the system if there is no viable framebuffer to use, so a framebuffer is guaranteed to be
	// present.
	u32* framebuffer = g_bootInfo.Framebuffer;

	// MemoryFill(framebuffer, 0, g_bootInfo.FramebufferSize);
	// We don't know if the COM1 serial output device has been initialized in any way, so we initialize it here ourselves
	PanicReinitializeSerialConsole();

	PanicClearScreen(framebuffer);

	PanicWriteString("!!!UNRECOVERABLE KERNEL PANIC!!!\n", framebuffer, &cursorPositionX, &cursorPositionY);

	// Print out the file name and number on which the panic occured
	PanicWriteString("Occured at: ", framebuffer, &cursorPositionX, &cursorPositionY);
	PanicWriteString(fileName, framebuffer, &cursorPositionX, &cursorPositionY);
	PanicWriteChar(':', framebuffer, &cursorPositionX, &cursorPositionY);
	i8 lineNumberString[MAX_DEC_LENGTH];
	NumberToDecimalString(lineNumber, lineNumberString);
	PanicWriteString(lineNumberString, framebuffer, &cursorPositionX, &cursorPositionY);
	PanicWriteChar('\n', framebuffer, &cursorPositionX, &cursorPositionY);

	PanicWriteString(message, framebuffer, &cursorPositionX, &cursorPositionY);
	PanicWriteString("\n\n", framebuffer, &cursorPositionX, &cursorPositionY);
	PanicWriteString("Please restart your computer to continue.\n", framebuffer, &cursorPositionX, &cursorPositionY);
	PanicWriteString("Hanging...\n", framebuffer, &cursorPositionX, &cursorPositionY);

	Hang();
}
