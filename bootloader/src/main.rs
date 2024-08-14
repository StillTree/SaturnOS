#![no_std]
#![no_main]

use core::{arch::asm, panic::PanicInfo};

use kernel::KernelExecutable;
use logger::LOGGER;
use memory::NutcrackerFrameAllocator;
use uefi::{cstr16, entry, table::{boot::MemoryType, Boot, SystemTable}, Handle, Status};
use x86_64::{instructions::hlt, registers::control::Cr3, structures::paging::{FrameAllocator, Mapper, OffsetPageTable, Page, PageTable, PageTableFlags, PageTableIndex, PhysFrame, Size4KiB}, PhysAddr, VirtAddr};

mod logger;
mod memory;
mod kernel;
mod esp_file_system;

#[panic_handler]
fn panic_handler(info: &PanicInfo) -> ! {
    if let Some(logger) = LOGGER.get() {
        unsafe {
            logger.0.force_unlock();
        }

        log::error!("{}", info);
    } else {
        serial_println!("{}", info);
    }

    loop {
        hlt();
    }
}

#[entry]
fn efi_main(image: Handle, system_table: SystemTable<Boot>) -> Status {
    logger::init(system_table.boot_services());

    log::info!("Welcome to the NutCracker bootloader!");

    log::info!("cr3 address: {:?}", Cr3::read().0.start_address());

    let file_contents = esp_file_system::read_file(system_table.boot_services(), cstr16!("nutcracker\\test.txt"));
    log::info!("File contents: {}", core::str::from_utf8(file_contents).unwrap().trim_end());

    let kernel_file = esp_file_system::read_file(system_table.boot_services(), cstr16!("nutcracker\\kernel"));
    let kernel = KernelExecutable::parse(kernel_file);

    log::info!("Exitting boot services");
    let (_, mut memory_map) = unsafe { system_table.exit_boot_services(MemoryType::LOADER_DATA) };

    memory_map.sort();

    // TODO: Make this more secure by figuring out a way to iterate from the beginning no matter
    // what iterator is passed in.
    let mut frame_allocator = NutcrackerFrameAllocator::new(memory_map.entries());

    let (frame, table) = create_kernel_page_table(&mut frame_allocator);

    // TODO: Identity-map the VGA text buffer
    load_kernel(kernel, table, frame, frame_allocator);

    // Bro really debugged this shit for like 30 minutes just to find out,
    // that you can't return after exiting boot services 💀
    //Status::SUCCESS
}

fn load_kernel(
    mut kernel: KernelExecutable,
    mut kernel_page_table: OffsetPageTable,
    kernel_page_table_frame: PhysFrame,
    mut frame_allocator: NutcrackerFrameAllocator,
) -> ! {
    let entry_point = kernel.load_segments(&mut kernel_page_table, &mut frame_allocator);

    for lol in kernel_page_table.level_4_table().iter() {
        if lol.is_unused() {
            continue;
        }

        log::info!("{:#?}", lol);
    }

    // Getting the first L4 entry that's available getting its corresponding address
    // and creating the kernel's stack there. Its size is completely arbitrary and might change I guess
    let stack_size = 81920; // 80 KiB plus a guard page
    
    let l4_entry = kernel_page_table.level_4_table().iter().position(|entry| entry.is_unused())
        .expect("No available kernel page table entries found");

    let stack_start_addr = Page::from_page_table_indices_1gib(PageTableIndex::new(u16::try_from(l4_entry).unwrap()), PageTableIndex::new(0)).start_address() + 4096;
    let stack_end_addr = stack_start_addr + stack_size;
    
    log::info!("Kernel stack start at: {:?}", stack_start_addr);

    for page in Page::range_inclusive(Page::containing_address(stack_start_addr), Page::containing_address(stack_end_addr - 1)) {
        let frame = frame_allocator.allocate_frame().expect("Could not allocate a frame for the kernel's stack");
        log::info!("Mapping kernel stack page {:?} to frame {:?}", page.start_address(), frame.start_address());

        unsafe {
            kernel_page_table.map_to(
                page,
                frame,
                PageTableFlags::PRESENT | PageTableFlags::WRITABLE | PageTableFlags::NO_EXECUTE,
                &mut frame_allocator
            ).expect("Could not map a frame to a kernel stack page")
        }.ignore();
    }

    // Identity mapping the context switch function so we don't immediately page fault
    // when the kernel takes over
    let context_switch_fn_phys_frame: PhysFrame<Size4KiB> = PhysFrame::containing_address(PhysAddr::new(context_switch as *const () as u64));
    log::info!("Context switch function at: {:?}", context_switch_fn_phys_frame.start_address());

    for frame in PhysFrame::range_inclusive(context_switch_fn_phys_frame, context_switch_fn_phys_frame + 1) {
        let page: Page<Size4KiB> = Page::containing_address(VirtAddr::new(frame.start_address().as_u64()));

        unsafe {
            kernel_page_table.map_to_with_table_flags(
                page,
                frame,
                PageTableFlags::PRESENT,
                PageTableFlags::PRESENT | PageTableFlags::WRITABLE,
                &mut frame_allocator
            ).expect("Could not identity map context switch function frame")
        }.ignore();
    }

    context_switch(kernel_page_table_frame.start_address(), stack_end_addr, entry_point);
}

fn context_switch(page_table_addr: PhysAddr, stack_beginning_addr: VirtAddr, entry_point: VirtAddr) -> ! {
    unsafe {
        asm!(
            r#"
            xor rbp, rbp
            mov cr3, {}
            mov rsp, {}
            push 0
            jmp {}
            "#,
            in(reg) page_table_addr.as_u64(),
            in(reg) stack_beginning_addr.as_u64(),
            in(reg) entry_point.as_u64()
        );
    };

    unreachable!();
}

fn create_kernel_page_table(frame_allocator: & mut NutcrackerFrameAllocator) -> (PhysFrame, OffsetPageTable<'static>) {
    let new_frame = frame_allocator.allocate_frame().expect("No unused pages available");
    log::info!("New kernel L4 page table: {:#?}", new_frame);

    let ptr = VirtAddr::new(new_frame.start_address().as_u64()).as_mut_ptr();
    unsafe {
        *ptr = PageTable::new();
    };

    (new_frame, unsafe { OffsetPageTable::new(&mut *ptr, VirtAddr::new(0)) })
}

