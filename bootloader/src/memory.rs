use core::iter::Copied;

use uefi::table::boot::{MemoryDescriptor, MemoryMapIter, MemoryType};
use x86_64::{structures::paging::{FrameAllocator, PhysFrame, Size4KiB}, PhysAddr};

/// A convenience function for checking if a descriptor is usable.
#[inline]
fn memory_descriptor_usable(memory_descriptor: &MemoryDescriptor) -> bool {
    memory_descriptor.ty == MemoryType::CONVENTIONAL
}

/// Returns the next available frame from the given descriptor starting with `last_frame` (exclusive).
fn next_frame_from_descriptor(memory_descriptor: &MemoryDescriptor, last_frame: PhysFrame) -> Option<PhysFrame> {
    let min_frame: PhysFrame<Size4KiB> = PhysFrame::containing_address(PhysAddr::new(memory_descriptor.phys_start));
    let max_frame: PhysFrame<Size4KiB> = PhysFrame::containing_address(PhysAddr::new(memory_descriptor.phys_start + memory_descriptor.page_count * 4096) - 1);

    // If the last allocated frame is outside the bounds of the memory descriptor,
    // we know that the first frame will be available so we return it.
    if last_frame < min_frame {
        return Some(min_frame);
    }

    // Check if there is one more frame to allocate
    if last_frame < max_frame {
        return Some(last_frame + 1);
    }

    None
}

/// We don't talk about what's below it...
const MIN_PHYS_MEMORY_ADDRESS: PhysAddr = PhysAddr::new(10_000);

/// A sequential physical frame allocator.
pub struct NutcrackerFrameAllocator<'a> {
    memory_map_entries: Copied<MemoryMapIter<'a>>,
    current_descriptor: MemoryDescriptor,
    previous_frame: PhysFrame,
}

impl<'a> NutcrackerFrameAllocator<'a> {
    /// Creates a new sequential physical frame allocator based on the copied entries
    /// from the given memory descriptor iterator.
    pub fn new(mut memory_map_entries: MemoryMapIter<'a>) -> Self {
        let current_descriptor = memory_map_entries.find(|descriptor| memory_descriptor_usable(descriptor))
            .expect("Could not find a usable memory region for the allocator");

        let memory_map_entries = memory_map_entries.copied();

        NutcrackerFrameAllocator {
            memory_map_entries,
            current_descriptor: current_descriptor.clone(),
            previous_frame: PhysFrame::containing_address(MIN_PHYS_MEMORY_ADDRESS),
        }
    }

    /// A helper function that allocates the next free frame from the current descriptor,
    /// only if they're available, otherwise returns `None`.
    fn allocate_next_current_descriptor_frame(&mut self) -> Option<PhysFrame> {
        let next_available_frame = next_frame_from_descriptor(&self.current_descriptor, self.previous_frame)?;

        // When returning the next available frame immediately becomes the previous one,
        // because it's being consumed.
        self.previous_frame = next_available_frame;
            
        Some(next_available_frame)
    }
}

unsafe impl<'a> FrameAllocator<Size4KiB> for NutcrackerFrameAllocator<'a> {
    fn allocate_frame(&mut self) -> Option<PhysFrame<Size4KiB>> {
        // If there is an available frame, return it.
        if let Some(frame) = self.allocate_next_current_descriptor_frame() {
            return Some(frame);
        }

        // If there are no available more available frames to be allocated from the current descriptor,
        // we need to find the next available memory descriptor. If its "usable", allocate a frame from it.
        while let Some(next_descriptor) = self.memory_map_entries.next() {
            if !memory_descriptor_usable(&next_descriptor) {
                continue;
            }

            if let Some(frame) = next_frame_from_descriptor(&next_descriptor, self.previous_frame) {
                self.current_descriptor = next_descriptor;

                return Some(frame);
            }
        }

        None
    }
}

