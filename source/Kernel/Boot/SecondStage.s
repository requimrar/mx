// SecondStage.s
// Copyright (c) 2011-2013, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


.section .text

.global KernelBootstrap
.global GetReturnAddress

.type KernelBootstrap, @function
.type GetReturnAddress, @function

.code64



KernelBootstrap:
	movq $0x60000, %rsp




	mov $0, %rbp


	// in boot.s we memcpy()'d the boot struct to 0x40000 to avoid trashing it
	mov $0x40000, %ebx
	mov 0x0500, %eax		// magic number

	movl $0x40200, 48(%ebx)



	// Push MBT struct pointer
	mov %rbx, %rsi

	// Push Magic value (0x2BADB002)
	mov %rax, %rdi







	// setup MSRs for syscall/sysret

	// modify STAR
	mov $0xC0000081, %ecx
	rdmsr
	// msr is edx:eax

	// simultaneously setup sysret CS and syscall CS
	mov $0x001B0008, %edx
	xor %eax, %eax
	wrmsr


	// now we modify LSTAR to hold the address of HandleSyscall
	xor %edx, %edx

	// TODO: Write handler for SYSCALL instruction instead of interrupt
	// keep both options available.
	// fill in address when ready
	// mov $HandleSyscallInstruction, %eax
	mov $0x00, %eax
	mov $0xC0000082, %ecx
	wrmsr


	// set SFMASK to 0.
	mov $0xC0000084, %ecx
	xor %eax, %eax
	wrmsr


	mov GDT64Pointer, %rax
	lgdtq GDT64Pointer


	// Set up segments (again)
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %gs

	mov $0x2B, %ax
	mov %ax, %fs


	mov $0x30, %ax
	ltr %ax



	mov $0x2B, %ax
	mov %ax, %fs



	// Call our kernel init -- this sets up the most basic services, but doesn't
	// initialise the threads.


	// some time ago, %r14 contains cursorX, %r15 contains cursorY
	// make these the third and fourth args.
	mov %r14, %rdx
	mov %r15, %rcx

	call KernelInit


	// call the kernel thread setup.
	call KernelThreadInit



	cli
halt:
	hlt
	jmp halt











.section .data
.align 16

// some C++ destructor shit
.global __dso_handle
__dso_handle:
		.quad	0



// 64-bit GDT
.global GDT64Pointer

GDT64:
	GDTNull:
		.word 0			// Limit (low)
		.word 0			// Base (low)
		.byte 0			// Base (middle)
		.byte 0			// Access
		.byte 0			// Granularity / Limit (high)
		.byte 0			// Base (high)
	GDTCode:
		.word 0xFFFF		// Limit (low)
		.word 0			// Base (low)
		.byte 0			// Base (middle)
		.byte 0x9A		// Access
		.byte 0xAF		// Granularity / Limit (high)
		.byte 0			// Base (high)
	GDTData:
		.word 0xFFFF		// Limit (low)
		.word 0			// Base (low)
		.byte 0			// Base (middle)
		.byte 0x92		// Access
		.byte 0xAF		// Granularity / Limit (high)
		.byte 0			// Base (high)
	GDTCodeR3:
		.word 0xFFFF		// Limit (low)
		.word 0			// Base (low)
		.byte 0			// Base (middle)
		.byte 0xFA		// Access
		.byte 0xAF		// Granularity / Limit (high)
		.byte 0			// Base (high)
	GDTDataR3:
		.word 0xFFFF		// Limit (low)
		.word 0			// Base (low)
		.byte 0			// Base (middle)
		.byte 0xF2		// Access
		.byte 0xAF		// Granularity / Limit (high)
		.byte 0			// Base (high)
	GDTFS:
		.word 0xFFFF		// Limit (low)
		.word 0			// Base (low)
		.byte 0			// Base (middle)
		.byte 0xF2		// Access
		.byte 0xAF		// Granularity (4) / Limit (high) (4)
		.byte 0			// Base (high)
	GDTTSS:
		.word 0x0068		// Limit (low)
		.word 0x2500		// Base (Addr of TSS)
		.byte 0x00		// middle
		.byte 0xE9
		.byte 0x80
		.byte 0x00
		.long 0x00
		.long 0x00



		// Pointer
	GDT64Pointer:
		.word GDT64Pointer - GDT64 - 1	// Limit
		.quad GDT64				// Base

