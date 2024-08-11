#![no_std]
#![no_main]

use core::panic::PanicInfo;

use kernel::KernelExecutable;
use logger::LOGGER;
use memory::NutcrackerFrameAllocator;
use uefi::{cstr16, entry, table::{boot::MemoryType, Boot, SystemTable}, Handle, Status};
use x86_64::{instructions::hlt, registers::control::Cr3, structures::paging::{FrameAllocator, OffsetPageTable, PageTable}, VirtAddr};

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

    let table = create_kernel_page_table(&mut frame_allocator);

    loop {
        hlt();
    }

    // Bro really debugged this shit for like 30 minutes just to find out,
    // that you can't return after exiting boot services 💀
    //Status::SUCCESS
}

fn create_kernel_page_table<'a>(frame_allocator: &'a mut NutcrackerFrameAllocator) -> OffsetPageTable<'a> {
    let new_frame = frame_allocator.allocate_frame().expect("No unused pages available");
    log::info!("New kernel L4 page table: {:#?}", new_frame);

    let ptr = VirtAddr::new(new_frame.start_address().as_u64()).as_mut_ptr();
    unsafe {
        *ptr = PageTable::new();
    };

    unsafe { OffsetPageTable::new(&mut *ptr, VirtAddr::new(0)) }
}

