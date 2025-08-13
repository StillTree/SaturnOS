.global SyscallHandler

.extern ProcessTerminateStart
.extern ProcessTerminateFinish
.extern ScheduleSyscallStart
.extern ScheduleSyscallFinish
.extern g_syscallFunctions
.extern g_scheduler

.equ CURRENT_THREAD_OFFSET, 128
.equ THREAD_RSP, 168
.equ THREAD_KERNEL_STACK_TOP, 192
.equ THREAD_PARENT_PROCESS, 200

SyscallHandler:
	pushq %rcx
	pushq %r11
	pushq %rbx

	lea g_scheduler(%rip), %rbx
	movq CURRENT_THREAD_OFFSET(%rbx), %rbx

	movq %rsp, THREAD_RSP(%rbx)
	movq THREAD_KERNEL_STACK_TOP(%rbx), %rsp

	cmp $0, %rax
	jne .NoTerminateSelf
	cmp $0, %rdi
	jne .NoTerminateSelf

	// This is a special case scenario where the calling process is terminating itself,
	// in my understanding there is no valid way to context switch to another process with sysretq,
	// so I prepare a full interrupt frame that my scheduler will handle

	// R12 contains the pointer to the thread that will be terminated
	lea g_scheduler(%rip), %r12
	movq CURRENT_THREAD_OFFSET(%r12), %r12
	movq THREAD_PARENT_PROCESS(%r12), %r12
	movq %r12, %rdi

	call ProcessTerminateStart

	call ScheduleSyscallStart

	lea g_scheduler(%rip), %rbx
	movq CURRENT_THREAD_OFFSET(%rbx), %rbx
	movq THREAD_KERNEL_STACK_TOP(%rbx), %rsp

	// We now have a valid stack and can context switch properly
	movq %r12, %rdi
	call ProcessTerminateFinish

	sub $168, %rsp // sizeof(CPUContext)

	mov %rsp, %rdi
	call ScheduleSyscallFinish

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

.NoTerminateSelf:
	cmp $2, %rax
	jae .Error

	movq %r10, %rcx
	lea g_syscallFunctions(%rip), %rbx
	movq (%rbx, %rax, 8), %rbx
	call *%rbx

	lea g_scheduler(%rip), %rbx
	movq CURRENT_THREAD_OFFSET(%rbx), %rbx
	movq THREAD_RSP(%rbx), %rsp

	popq %rbx
	popq %r11
	popq %rcx

	sysretq

.Error:
	// 25 - ResultInvalidSyscallNumber
	mov $25, %rax

	lea g_scheduler(%rip), %rbx
	movq CURRENT_THREAD_OFFSET(%rbx), %rbx
	movq THREAD_RSP(%rbx), %rsp

	popq %rbx
	popq %r11
	popq %rcx

	sysretq
