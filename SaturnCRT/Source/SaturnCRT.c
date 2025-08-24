#include "Core.h"

extern u64 Main();

void _start()
{
	// For now return value ignored
	// TODO: Return value support
	Main();

	__asm__ volatile("movq $0, %rax\n"
					 "movq $0, %rdi\n"
					 "syscall");
}
