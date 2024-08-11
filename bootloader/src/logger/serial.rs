use lazy_static::lazy_static;
use uart_16550::SerialPort;

// Lazily initialize the serial port so it is available right after launch
// Not using UEFI to communicate to make it available after exitting boot services
// and I also am to lazy to write all that necessary code to make it work
lazy_static! {
    pub static ref SERIAL1: spin::Mutex<SerialPort> = {
        let mut serial_port = unsafe { SerialPort::new(0x3f8) };
        serial_port.init();
        spin::Mutex::new(serial_port)
    };
}

#[doc(hidden)]
pub fn _print(args: ::core::fmt::Arguments) {
    use core::fmt::Write;
    use x86_64::instructions::interrupts;

    interrupts::without_interrupts(|| {
        SERIAL1.lock().write_fmt(args).expect("Printing to serial failed");
    });
}

/// Writes the formatted data to the `0x3f8` serial port.
#[macro_export]
macro_rules! serial_print {
    ($($arg:tt)*) => {
        $crate::logger::serial::_print(format_args!($($arg)*));
    };
}

/// Writes the formatted data to the `0x3f8` serial port, with a newline appended.
#[macro_export]
macro_rules! serial_println {
    () => ($crate::serial_print!("\n"));
    ($fmt:expr) => ($crate::serial_print!(concat!($fmt, "\n")));
    ($fmt:expr, $($arg:tt)*) => ($crate::serial_print!(
        concat!($fmt, "\n"), $($arg)*));
}

