use core::{fmt::{self, Write}, ptr::read_volatile};

use conquer_once::spin::OnceCell;
use noto_sans_mono_bitmap::{get_raster, get_raster_width, FontWeight, RasterHeight};
use spinning_top::Spinlock;
use uefi::proto::console::gop::PixelFormat;

pub static LOGGER: OnceCell<NutcrackerLogger> = OnceCell::uninit();

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

pub struct FramebufferInfo {
    pub stride: usize,
    pub pixel_format: PixelFormat,
    pub width: usize,
}

pub struct FramebufferLogger {
    framebuffer: &'static mut [u8],
    graphics_mode_info: FramebufferInfo,
    cursor_position_x: usize,
    cursor_position_y: usize,
}

impl FramebufferLogger {
    pub fn new(buffer: &'static mut [u8], info: FramebufferInfo) -> Self {
        FramebufferLogger {
            framebuffer: buffer,
            graphics_mode_info: info,
            cursor_position_x: DEFAULT_BORDER_PADDING,
            cursor_position_y: DEFAULT_BORDER_PADDING,
        }
    }

    fn set_pixel(&mut self, x: usize, y: usize, red: u8, green: u8, blue: u8) {
        let data_offset = (y * self.graphics_mode_info.stride + x) * 4;

        let data = match self.graphics_mode_info.pixel_format {
            PixelFormat::Rgb => [red, green, blue, 0],
            PixelFormat::Bgr => [blue, green, red, 0],
            _ => {
                panic!("The only supported pixel formats are RGB and BGR");
            }
        };

        self.framebuffer[data_offset..(data_offset + 4)].copy_from_slice(&data);
        // Perform a volatile read so the compiler won't optimize this away
        let _ = unsafe { read_volatile(&self.framebuffer[data_offset]) };
    }

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
                    for (x, data) in row.iter().enumerate() {
                        self.set_pixel(self.cursor_position_x + x, self.cursor_position_y + y, *data, *data, *data);
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

