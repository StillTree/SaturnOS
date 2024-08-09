#![no_std]
#![no_main]

use core::{panic::PanicInfo, slice::from_raw_parts_mut};

use framebuffer_log::{FramebufferInfo, FramebufferLogger, NutcrackerLogger, LOGGER};
use spinning_top::Spinlock;
use uefi::{cstr16, entry, prelude::BootServices, proto::{console::gop::GraphicsOutput, media::file::{File, FileAttribute, FileInfo, FileType}}, table::{boot::{AllocateType, MemoryType}, Boot, SystemTable}, CStr16, Handle, Status};
use x86_64::{instructions::hlt, registers::control::Cr3};

mod framebuffer_log;

#[panic_handler]
fn panic_handler(info: &PanicInfo) -> ! {
    unsafe { LOGGER.get().unwrap().0.force_unlock(); };

    log::error!("{}", info);

    loop {
        hlt();
    }
}

#[entry]
fn efi_main(image: Handle, mut system_table: SystemTable<Boot>) -> Status {
    init_framebuffer_logger(system_table.boot_services());

    log::info!("Welcome to the NutCracker bootloader!");

    log::info!("cr3 address: {:?}", Cr3::read().0.start_address());

    let file_contents = read_file(system_table.boot_services(), cstr16!("nutcracker\\test.txt"));

    log::info!("File contents: {}", core::str::from_utf8(file_contents).unwrap().trim_end());

    log::info!("Exiting boot services in 5 seconds...");
    
    system_table.boot_services().stall(60_000_000);

    let (_, _) = unsafe { system_table.exit_boot_services(MemoryType::LOADER_DATA) };

    loop {
        hlt();
    }

    // Bro really debugged this shit for like 30 minutes just to find out,
    // that you can't return after exiting boot services 💀
    //Status::SUCCESS
}

fn init_framebuffer_logger<'a>(boot_services: &'a BootServices) {
    let graphics_out_handle = boot_services.get_handle_for_protocol::<GraphicsOutput>()
        .expect("Could not get GraphicsOutput protocol handle");

    let mut graphics_out = boot_services.open_protocol_exclusive::<GraphicsOutput>(graphics_out_handle)
        .expect("Could not open the GraphicsOutput protocol");

    let mode = graphics_out.modes(boot_services)
        .filter(|mode| mode.info().resolution().0 >= 800 && mode.info().resolution().1 >= 600)
        .nth(0) // Pick the first one found
        .expect("Could not find a suitable GraphicsOutput mode");

    graphics_out.set_mode(&mode).expect("Could not set a GraphicsOutput mode");
    let mode_info = graphics_out.current_mode_info();

    let mut framebuffer = graphics_out.frame_buffer();

    framebuffer_log::LOGGER.get_or_init(move || {
        NutcrackerLogger(
            Spinlock::new(
                FramebufferLogger::new(
                    unsafe { from_raw_parts_mut(framebuffer.as_mut_ptr(), framebuffer.size()) },
                    FramebufferInfo {
                        width: mode_info.resolution().0,
                        stride: mode_info.stride(),
                        pixel_format: mode_info.pixel_format(),
                    },
                )
            )
        )
    });

    log::set_logger(LOGGER.get().unwrap()).expect("Another logger is already present");
    log::set_max_level(log::LevelFilter::Trace);
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
        // Round up the number of needed pages by adding 4095 to the needed size
        .allocate_pages(AllocateType::AnyPages, MemoryType::LOADER_DATA, (memory_size + 4095) / 4096)
        .unwrap() as *mut u8;

    // Memset the whole thing to 0s and create a mutable slice from it
    unsafe {
        core::ptr::write_bytes(content_ptr, 0, memory_size);

        core::slice::from_raw_parts_mut(content_ptr, memory_size)
    }
}

