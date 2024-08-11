use xmas_elf::ElfFile;

pub struct KernelExecutable<'a> {
    pub elf: ElfFile<'a>,
}

impl<'a> KernelExecutable<'a> {
    pub fn parse(data: &'a [u8]) -> Self {
        let elf = ElfFile::new(data)
            .expect("Failed to parse the ELF kernel executable");

        KernelExecutable {
            elf
        }
    }
}

