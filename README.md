# Saturn OS

This is a repository (monorepo?) for storing all necessary components for runnning my own hobby operating system.

> Compiled and tested using Clang, other compilers will most definitely require some minor code changes.

## Structure

```
- Kernel (SaturnKernel)
- Bootloader (Supernova)
- README.md
- LICENSE (GPLv3)
- titan (a utility build tool based on make)
- Makefile (calls all the other Makefiles)
...
```

Currently there are 3 main components:
-   The kernel itself - `SaturnKernel`
-   The bootloader - `Supernova`
-   The utility build script - `Titan`

## Custom toolchain

I use a completely custom OS-specific Clang toolchain. All of the instructions on correctly building all of its parts
can be found [here](ToolChain/README.md).

## Roadmap

More like plans for the future but yes :D

- do some shit: elf loading, a build chain, syscalls, loading programs, etc.
- adding more ext2 support into the monolith

- improve the scheduler, maybe rewrite for ipc in mind
- a robust ipc system inspired by the zircon/fuchsia message passing
- move stuff out of the kernel into separate processes
- do some more synchronisation so everything is safe
- eventually the only syscalls left will be the ones for message passing, everything else will be done by the message passing,
e.g. to read a file a process would need to send a message to the vfs process and the vfs process will then respond

These are vague plans for me split into two "phases". Phase one: fun shit - just implement stuff and gain knowledge and experience.
Phase two: my IPC dream - I don't know if that can be calld a microkernel, but that is my dream kernel vision, a micro-vision perhaps :D,
that will take a lot of time and planning but I will eventually move towards that goal, slowly but steadily.

## Why?

Well, why not? I am trying to learn low-level programming and always wanted to create myself an OS, so this is perfect :D.
