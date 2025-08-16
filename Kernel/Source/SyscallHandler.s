.global SyscallHandler

.extern ScheduleProcessTerminate
.extern g_syscallFunctions
.extern g_scheduler

.equ CURRENT_THREAD_OFFSET, 128
.equ THREAD_RSP, 168
.equ THREAD_KERNEL_STACK_TOP, 192

SyscallHandler:
	pushq %rcx
	pushq %r11
	pushq %rbx

	// Switching to the currently running thread's kernel stack
	lea g_scheduler(%rip), %rbx
	movq CURRENT_THREAD_OFFSET(%rbx), %rbx

	movq %rsp, THREAD_RSP(%rbx)
	movq THREAD_KERNEL_STACK_TOP(%rbx), %rsp

	cmp $3, %rax
	jae .Error

	movq %r10, %rcx
	lea g_syscallFunctions(%rip), %rbx
	movq (%rbx, %rax, 8), %rbx
	call *%rbx

	// Switching back to the currently running thread's user stack
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

	// Switching back to the currently running thread's user stack
	lea g_scheduler(%rip), %rbx
	movq CURRENT_THREAD_OFFSET(%rbx), %rbx
	movq THREAD_RSP(%rbx), %rsp

	popq %rbx
	popq %r11
	popq %rcx

	sysretq
