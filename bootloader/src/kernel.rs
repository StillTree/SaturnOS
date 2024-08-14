use x86_64::{align_up, structures::paging::{mapper::{MappedFrame, TranslateResult}, FrameAllocator, Mapper, OffsetPageTable, Page, PageSize, PageTableFlags, PhysFrame, Size4KiB, Translate}, PhysAddr, VirtAddr};
use xmas_elf::{header, program, ElfFile};

use crate::memory::NutcrackerFrameAllocator;

pub struct KernelExecutable<'a> {
    pub elf: ElfFile<'a>,
}

impl<'a> KernelExecutable<'a> {
    pub fn parse(data: &'a [u8]) -> Self {
        let elf = ElfFile::new(data)
            .expect("Failed to parse the ELF kernel executable");

        header::sanity_check(&elf)
            .expect("Invalid ELF kernel executable header");

        for program_header in elf.program_iter() {
            program::sanity_check(program_header, &elf)
                .expect("Invalid program header in the ELF kernel exevutable");
        }

        if elf.header.pt2.type_().as_type() != header::Type::Executable {
            panic!("The ELF kernel executable file has an unexpected type {:?} instead of type Type::Executable", elf.header.pt2.type_().as_type());
        }

        KernelExecutable {
            elf,
        }
    }

    pub fn load_segments(&mut self, kernel_page_table: &mut OffsetPageTable, frame_allocator: &mut NutcrackerFrameAllocator) {
        for a in self.elf.program_iter() {
            log::info!("{:#?}", a);
        }
        // Handling all loadable segments
        for program_header in self.elf.program_iter() {
            if program_header.get_type().expect("Could not get ELF kernel executable program header type") != program::Type::Load {
                continue;
            }

            log::info!("Handling a Type::Load segment: {:#?}", program_header);

            let kernel_phys_address = PhysAddr::new(self.elf.input.as_ptr() as u64);
            let segment_phys_address = kernel_phys_address + program_header.offset();
            let segment_phys_frame: PhysFrame<Size4KiB> = PhysFrame::containing_address(segment_phys_address);
            let segment_phys_end_frame: PhysFrame<Size4KiB> = PhysFrame::containing_address(segment_phys_address + program_header.file_size() - 1);

            let segment_virt_address = VirtAddr::new(program_header.virtual_addr());
            let segment_virt_page: Page<Size4KiB> = Page::containing_address(segment_virt_address);

            let mut flags = PageTableFlags::PRESENT;
            if !program_header.flags().is_execute() {
                flags |= PageTableFlags::NO_EXECUTE;
            }
            if program_header.flags().is_write() {
                flags |= PageTableFlags::WRITABLE;
            }

            log::info!("Segment flags: {:?}", flags);

            for frame in PhysFrame::range_inclusive(segment_phys_frame, segment_phys_end_frame) {
                let page = segment_virt_page + (frame - segment_phys_frame);
                log::info!("Mapping page at address {:?} to frame {:?}", page.start_address(), frame.start_address());

                unsafe {
                    kernel_page_table.map_to_with_table_flags(
                        page,
                        frame,
                        flags,
                        PageTableFlags::PRESENT| PageTableFlags::WRITABLE,
                        frame_allocator
                    ).expect("Could not map a frame to the kernel page table")
                }.ignore();
            }

            // This is a .bss section and must be handled accordingly
            if program_header.mem_size() > program_header.file_size() {
                // Completely ripped line for line from the Rust bootloader crate, I have no clue
                // what's happening here. Later I'll rewrite this maybe, we'll see.
                let zero_start = segment_virt_address + program_header.file_size();
                let zero_end = segment_virt_address + program_header.mem_size();

                type PageArray = [u64; Size4KiB::SIZE as usize / 8];
                const ZERO_ARRAY: PageArray = [0; Size4KiB::SIZE as usize / 8];

                let data_bytes_before_zero = zero_start.as_u64() & 0xfff;
                if data_bytes_before_zero != 0 {
                    let last_page: Page<Size4KiB> = Page::containing_address(segment_virt_address + program_header.file_size() - 1);

                    // Make mut
                    let (frame, flags) = match kernel_page_table.translate(last_page.start_address()) {
                        TranslateResult::Mapped { frame, offset: _, flags } => (frame, flags),
                        _ => panic!("The kernel ELF executable has not been mapped correctly to memory"),
                    };

                    let frame = if let MappedFrame::Size4KiB(frame) = frame {
                        frame
                    } else {
                        panic!("Only 4K pages are supported");
                    };
                    
                    let new_frame = frame_allocator.allocate_frame().unwrap();
                    let frame_ptr = frame.start_address().as_u64() as *const u8;
                    let new_frame_ptr = new_frame.start_address().as_u64() as *mut u8;
                    unsafe {
                        core::ptr::copy_nonoverlapping(frame_ptr, new_frame_ptr, Size4KiB::SIZE as usize);
                    };

                    kernel_page_table.unmap(last_page).unwrap().1.ignore();
                    unsafe {
                        kernel_page_table.map_to(last_page, new_frame, flags, frame_allocator).unwrap().ignore();
                    };

                    let new_bytes_ptr = new_frame.start_address().as_u64() as *mut u8;

                    unsafe {
                        core::ptr::write_bytes(new_bytes_ptr.add(data_bytes_before_zero as usize), 0, (Size4KiB::SIZE - data_bytes_before_zero) as usize);
                    };
                }

                let start_page: Page<Size4KiB> = Page::containing_address(VirtAddr::new(align_up(zero_start.as_u64(), Size4KiB::SIZE)));
                let end_page: Page<Size4KiB> = Page::containing_address(zero_end - 1);
                for page in Page::range_inclusive(start_page, end_page) {
                    let frame = frame_allocator.allocate_frame().unwrap();

                    let frame_ptr = frame.start_address().as_u64() as *mut PageArray;
                    unsafe { frame_ptr.write(ZERO_ARRAY) };

                    unsafe {
                        kernel_page_table.map_to_with_table_flags(
                            page,
                            frame,
                            flags,
                            PageTableFlags::PRESENT | PageTableFlags::WRITABLE,
                            frame_allocator,
                        ).expect("Could not write page table")
                    }.ignore();
                }
            }
        }

        // Handle the Relocation Read-Only segments
        for program_header in self.elf.program_iter() {
            if program_header.get_type().expect("Could not get ELF kernel executable program header type") != program::Type::GnuRelro {
                continue;
            }

            log::info!("Handling a Type::GnuRelro segment: {:#?}", program_header);
            let segment_virt_address = VirtAddr::new(program_header.virtual_addr());
            let segment_virt_end_address = VirtAddr::new(segment_virt_address.as_u64() + program_header.mem_size());

            let segment_virt_page: Page<Size4KiB> = Page::containing_address(segment_virt_address);
            let segment_virt_end_page: Page<Size4KiB> = Page::containing_address(segment_virt_end_address - 1);

            log::info!("Fist segment page: {:?}", segment_virt_page.start_address());

            for page in Page::range_inclusive(segment_virt_page, segment_virt_end_page) {
                log::info!("Translating a page at the address {:?}", page.start_address());
                let res = kernel_page_table.translate(page.start_address());

                let flags = match res {
                    TranslateResult::Mapped { frame: _, offset: _, flags } => flags,
                    _ => panic!("The kernel ELF executable has not been mapped correctly to memory"),
                };

                log::info!("Page flags: {:?}", flags);

                if flags.contains(PageTableFlags::WRITABLE) {
                    unsafe {
                        kernel_page_table.update_flags(page, flags & !PageTableFlags::WRITABLE)
                            .expect("Could not update flags in the kernel page table")
                            .ignore();
                    };
                }
            }
        }
    }
}

