.global ScheduleInterruptHandler

.extern EOISignal
.extern Schedule

ScheduleInterruptHandler:
	// Already present on the stack thanks to the CPU :D
	// push %ss
	// push %rsp
	// push %rflags
	// push %cs
	// push %rip

	// push %gs
	// push %fs
	push %r15
	push %r14
	push %r13
	push %r12
	push %r11
	push %r10
	push %r9
	push %r8
	push %rbp
	push %rdi
	push %rsi
	push %rdx
	push %rcx
	push %rbx
	push %rax
	mov %cr3, %rax
	push %rax

	mov %rsp, %rdi
	call Schedule

	call EOISignal

	pop %rax
	mov %rax, %cr3
	pop %rax
	pop %rbx
	pop %rcx
	pop %rdx
	pop %rsi
	pop %rdi
	pop %rbp
	pop %r8
	pop %r9
	pop %r10
	pop %r11
	pop %r12
	pop %r13
	pop %r14
	pop %r15
	// pop %fs
	// pop %gs

	// Restored by the CPU :D
	// pop %rip
	// pop %cs
	// pop %rflags
	// pop %rsp
	// pop %ss

	iretq
