use core::{fmt::{self, Write}, ptr::read_volatile, slice::from_raw_parts_mut};

use conquer_once::spin::OnceCell;
use noto_sans_mono_bitmap::{get_raster, get_raster_width, FontWeight, RasterHeight};
use spinning_top::Spinlock;
use uefi::{prelude::BootServices, proto::console::gop::{GraphicsOutput, PixelFormat}};

/// The global logger instance uninitialized until the `init` function gets called.
pub static LOGGER: OnceCell<NutcrackerLogger> = OnceCell::uninit();

/// Initialize the global logger using a framebuffer and the `GraphicsOutput` protocol.
pub fn init<'a>(boot_services: &'a BootServices) {
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

    LOGGER.get_or_init(move || {
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

    // The logger is guaranteed to be set so `unwrap` shouldn't cause any unexpected panics
    log::set_logger(LOGGER.get().unwrap()).expect("Another logger is already present");
    log::set_max_level(log::LevelFilter::Trace);
}

/// A wrapper type that implements the `log::Log` trait. It is necessary because
/// the trait doesn't allow a mutable borrow in the `log`'s function signature,
/// which is necessary to write data to the framebuffer.
pub struct NutcrackerLogger(pub Spinlock<FramebufferLogger>);

impl log::Log for NutcrackerLogger {
    fn enabled(&self, _meta: &log::Metadata) -> bool {
        true
    }

    fn flush(&self) {}

    fn log(&self, record: &log::Record) {
        let mut logger = self.0.lock();

        writeln!(logger, "[{:5}]: {}", record.level(), record.args()).unwrap();
    }
}

const DEFAULT_FONT_HEIGHT: RasterHeight = RasterHeight::Size20;
const DEFAULT_FONT_WIDTH: usize = get_raster_width(FontWeight::Regular, DEFAULT_FONT_HEIGHT);
const DEFAULT_CHAR_SPACING: usize = 0;
const DEFAULT_LINE_SPACING: usize = 1;
const DEFAULT_BORDER_PADDING: usize = 2;

/// The minimal required information for the framebuffer to function.
pub struct FramebufferInfo {
    pub stride: usize,
    pub pixel_format: PixelFormat,
    pub width: usize,
}

/// The actual logger and its functionality.
pub struct FramebufferLogger {
    framebuffer: &'static mut [u8],
    graphics_mode_info: FramebufferInfo,
    cursor_position_x: usize,
    cursor_position_y: usize,
}

impl FramebufferLogger {
    /// Creates a new instance with the provided framebuffer, information about it
    /// and default initial cursor position.
    pub fn new(buffer: &'static mut [u8], info: FramebufferInfo) -> Self {
        FramebufferLogger {
            framebuffer: buffer,
            graphics_mode_info: info,
            cursor_position_x: DEFAULT_BORDER_PADDING,
            cursor_position_y: DEFAULT_BORDER_PADDING,
        }
    }

    /// Sets a pixel with a given color at the specified framebuffer's position.
    fn set_pixel(&mut self, x: usize, y: usize, intensity: u8) {
        let data_offset = (y * self.graphics_mode_info.stride + x) * 4;

        let data = match self.graphics_mode_info.pixel_format {
            PixelFormat::Rgb | PixelFormat::Bgr => [intensity, intensity, intensity, 0],
            _ => panic!("The only supported pixel formats are RGB and BGR"),
        };

        // Copy the provided data to the specified position
        self.framebuffer[data_offset..(data_offset + 4)].copy_from_slice(&data);
        // Perform a volatile read so the compiler won't optimize this away
        let _ = unsafe { read_volatile(&self.framebuffer[data_offset]) };
    }

    /// Inserts the specified character at the cursor's current position
    /// and advances the cursor to the next position.
    pub fn write_char(&mut self, character: char) {
        match character {
            '\n' => {
                self.cursor_position_y += DEFAULT_FONT_HEIGHT.val() + DEFAULT_LINE_SPACING;
                self.cursor_position_x = DEFAULT_BORDER_PADDING;
            },
            '\r' => {
                self.cursor_position_x = DEFAULT_BORDER_PADDING;
            },
            character => {
                if self.cursor_position_x + DEFAULT_FONT_WIDTH + DEFAULT_CHAR_SPACING >= self.graphics_mode_info.width {
                    self.write_char('\n');
                }

                let raster = get_raster(character, FontWeight::Regular, DEFAULT_FONT_HEIGHT)
                    .unwrap_or(get_raster('�', FontWeight::Regular, DEFAULT_FONT_HEIGHT).unwrap());

                for (y, row) in raster.raster().iter().enumerate() {
                    for (x, intensity) in row.iter().enumerate() {
                        self.set_pixel(self.cursor_position_x + x, self.cursor_position_y + y, *intensity);
                    }
                }

                self.cursor_position_x += DEFAULT_FONT_WIDTH + DEFAULT_CHAR_SPACING;
        
            },
        };
    }
}

impl fmt::Write for FramebufferLogger {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        for c in s.chars() {
            self.write_char(c);
        }

        Ok(())
    }
}

