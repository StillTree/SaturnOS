/// C linking so the linker doesn't absolutely shit itself
extern "C" void KernelMain()
{
	while(true)
	{
		__asm__("cli; hlt");
	}
}

