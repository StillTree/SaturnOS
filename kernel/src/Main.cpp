/// C linking so the linker doesn't absolutely shit itself
extern "C" void KernelMain()
{
	int i = 55;
	i += 13;
	i -= 3;

	__asm__ volatile("outb %b0, %w1" : : "a"(i), "Nd"(0x3f8) : "memory");

	while(true)
	{
		__asm__("cli; hlt");
	}
}

