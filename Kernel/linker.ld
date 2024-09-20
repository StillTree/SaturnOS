OUTPUT_FORMAT(elf64-x86-64)

ENTRY(KernelMain)

SECTIONS
{
	KERNEL_VIRTUAL_START  = 0xffffffff80010000;
	. = KERNEL_VIRTUAL_START;

	.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRTUAL_START)
	{
		*(.text*)
		*(.gnu.linkonce.t*)
	}

	.data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRTUAL_START)
	{
		*(.data*)
		*(.gnu.linkonce.d*)
	}

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_START)
	{
		*(.rodata*)
		*(.gnu.linkonce.r*)
	}

	.bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRTUAL_START)
	{
		*(COMMON)
		*(.bss*)
		*(.gnu.linkonce.b*)
	}

	/DISCARD/ :
	{
		*(.ctors)
		*(.dtors)
		*(.comment)
		*(.eh_frame)
		*(.note.gnu.build-id)
	}
}

