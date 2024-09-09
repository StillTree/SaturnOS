OUTPUT_FORMAT(elf64-x86-64)

ENTRY(KernelMain)

SECTIONS
{
	KERNEL_PHYSICAL = 0x100000;
	KERNEL_VIRTUAL = 0xffffffff80000000;
	. = KERNEL_PHYSICAL + KERNEL_VIRTUAL;
	KERNEL_START = .;

	.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRTUAL)
	{
		TEXT_START = .;
		*(.text*)
		*(.gnu.linkonce.t*)
		TEXT_END = .;
	}

	.data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRTUAL)
	{
		DATA_START = .;
		*(.data*)
		*(.gnu.linkonce.d*)
		DATA_END = .;
	}

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRTUAL)
	{
		READ_ONLY_DATA_START = .;
		*(.rodata*)
		*(.gnu.linkonce.r*)
		READ_ONLY_DATA_END = .;
	}

	.bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRTUAL)
	{
		BLOCK_STARTING_SYMBOLS_START = .;
		*(COMMON)
		*(.bss*)
		*(.gnu.linkonce.b*)
		BLOCK_STARTING_SYMBOLS_END = .;
	}
	KERNEL_END = .;

	/DISCARD/ :
	{
		*(.ctors)
		*(.dtors)
		*(.comment)
		*(.eh_frame)
		*(.note.gnu.build-id)
	}
}

