struct KernelBootInfo
{
	unsigned long long framebufferAddress;
	unsigned long long framebufferSize;
	unsigned long long framebufferWidth;
	unsigned long long framebufferHeight;
};

/// C linking so the linker doesn't absolutely shit itself
extern "C" void KernelMain(KernelBootInfo* bootInfo)
{
	__asm__ volatile("outb %b0, %w1" : : "a"('A'), "Nd"(0x3f8) : "memory");

	unsigned int* framebuffer = (unsigned int*) bootInfo->framebufferAddress;
	for(int y = 0; y < 100; y++)
	{
		for(int x = 0; x < 100; x++)
		{
			framebuffer[y * bootInfo->framebufferWidth + x] = 0xff8800;
		}
	}

	while(true)
	{
		__asm__("cli; hlt");
	}
}

