#![no_std]
#![no_main]

use uefi::{cstr16, entry, prelude::BootServices, proto::media::file::{File, FileAttribute, FileInfo, FileType}, table::{boot::{AllocateType, MemoryType}, Boot, SystemTable}, CStr16, Handle, Status};

#[entry]
fn efi_main(image: Handle, mut system_table: SystemTable<Boot>) -> Status {
    uefi::helpers::init(&mut system_table).unwrap();

    log::info!("Welcome to the NutCracker bootloader!");

    let file_contents = read_file(system_table.boot_services(), cstr16!("nutcracker\\test.txt"));

    log::info!("File contents: {}", core::str::from_utf8(file_contents).unwrap());

    system_table.boot_services().stall(60_000_000); // 60 seconds

    Status::SUCCESS
}

/// Reads a file from the disk.
fn read_file<'a>(boot_services: &'a BootServices, file_name: &CStr16) -> &'a mut [u8] {
    let mut file_system = boot_services.get_image_file_system(boot_services.image_handle())
        .expect("Could not get the bootloader's image filesystem");

    let mut root_dir = file_system.open_volume()
        .expect("Could not open the filesystem's root directory");

    let file_handle = root_dir.open(file_name, uefi::proto::media::file::FileMode::Read, FileAttribute::empty())
        .expect("Could not open the provided file name");

    match file_handle.into_type().unwrap() {
        FileType::Dir(_) => panic!("The provided file name is a directory"),
        FileType::Regular(mut file) => {
            let mut buffer = [0; 128];
            let file_size = usize::try_from(file.get_info::<FileInfo>(&mut buffer).unwrap().file_size()).unwrap();

            let memory_slice = allocate_mem_pages(boot_services, file_size);

            file.read(memory_slice).unwrap();
        
            memory_slice
        }
    }
}

/// Returns a memory slice the size of `n * 4096` bytes (entire memory pages),
/// where `n` is the calculated number of pages needed, based on
/// the `memory_size` parameter.
fn allocate_mem_pages(boot_services: &BootServices, memory_size: usize) -> &mut [u8] {
    let content_ptr = boot_services
        .allocate_pages(AllocateType::AnyPages, MemoryType::LOADER_DATA, (memory_size + 4095) / 4096)
        .unwrap() as *mut u8;

    // Memset the whole thing to 0s and create a mutable slice from it
    unsafe {
        core::ptr::write_bytes(content_ptr, 0, memory_size);

        core::slice::from_raw_parts_mut(content_ptr, memory_size)
    }
}

