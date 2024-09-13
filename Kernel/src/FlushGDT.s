.globl FlushGDT

FlushGDT:
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	pop %rdi
	push $0x8
	push %rdi
	lretq

