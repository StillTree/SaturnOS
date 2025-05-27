.global SyscallHandler

.extern DispatchSyscall
.extern g_syscallFunctions

SyscallHandler:
	push %rcx
	push %r11
	push %rbx

	cmp $2, %rax
	jae .Error

	mov %r10, %rcx
	lea g_syscallFunctions(%rip), %rbx
	mov (%rbx, %rax, 8), %rbx
	call *%rbx

	pop %rbx
	pop %r11
	pop %rcx
	sysretq

.Error:
	// 25 - ResultInvalidSyscallNumber
	mov $25, %rax

	pop %rbx
	pop %r11
	pop %rcx
	sysretq
