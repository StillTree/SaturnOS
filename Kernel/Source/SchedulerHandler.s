.global ScheduleInterruptHandler
.global ScheduleProcessTerminate

.extern EOISignal
.extern ScheduleInterrupt
.extern ProcessTerminateStart
.extern ProcessTerminateFinish
.extern ScheduleDiscardStart
.extern ScheduleDiscardFinish
.extern g_scheduler

.equ CURRENT_THREAD_OFFSET, 128
.equ THREAD_KERNEL_STACK_TOP, 192
.equ THREAD_PARENT_PROCESS, 200

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
	call ScheduleInterrupt

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

ScheduleProcessTerminate:
	lea g_scheduler(%rip), %r12
	movq CURRENT_THREAD_OFFSET(%r12), %r12
	movq THREAD_PARENT_PROCESS(%r12), %r12
	movq %r12, %rdi
	call ProcessTerminateStart

	call ScheduleDiscardStart

	lea g_scheduler(%rip), %rbx
	movq CURRENT_THREAD_OFFSET(%rbx), %rbx
	movq THREAD_KERNEL_STACK_TOP(%rbx), %rsp

	// We now have a valid stack and can context switch properly
	movq %r12, %rdi
	call ProcessTerminateFinish

	sub $168, %rsp // sizeof(CPUContext)

	mov %rsp, %rdi
	call ScheduleDiscardFinish

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
