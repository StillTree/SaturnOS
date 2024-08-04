#![no_std]
#![no_main]

use uefi::{entry, table::{Boot, SystemTable}, Handle, Status};
use core::fmt::Write;

#[entry]
fn efi_main(_image: Handle, mut system_table: SystemTable<Boot>) -> Status {
    uefi::helpers::init(&mut system_table).unwrap();

    let stdout = system_table.stdout();
    stdout.clear().unwrap();
    writeln!(stdout, "Welcome to the NutCracker bootloader!").unwrap();
    writeln!(stdout, "Initializing...").unwrap();

    system_table.boot_services().stall(60_000_000);

    Status::SUCCESS
}
