# Saturn OS

This is a repository (monorepo?) for storing all necessary components for runnning my own hobby operating system.

> Compiled and tested using GCC, other compilers will most definitely require some minor code changes.

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

## Roadmap

More like plans for the future but yes :D

Up to now I have added a lot of stuff into the kernel which made everything a total mess. These are changes
(written down primarily for myself so I don't forget them ;D)that I plan to implement in the coming *units of time*
(I have no clue how long that will take):

-   Firstly, a scheduler to support multiple processes
-   A robust IPC system to be used primarily by th drivers
-   Next, breaking down the kernel into a hybrid design (drivers running as standalone processes and some other changes too, most likely)
-   Then, a VFS layer to support mounting different file systems and so on...
-   A rudamentary RAM Disk system in which the most necessary drivers will be placed
-   Using EDK II to build the bootloader (so as to not mess with my custom "UEFI headers")

### The driver design

I want it to somewhat resemble how macOS does these but in my, greatly simplified, way.

Each driver will be running as a separate process (in user space) with its own address space and so on.
When the kernel detects a device it checks if any drivers are available for that specific device type
(it will match these based on their system paths which will have a separate folder and etc. for a device type in PCI for example).
Then it will load the driver (the driver is stored as an elf executable with some special metadata) and initialize everything needed.
After that the kernel can communicate with the driver to use its functionality.

The communicating part will probably be handled by some sort of IPC or other similar system.

The drivers will of course have their own set of APIs and functions for use, provided by the kernel.

## Why?

Well, why not? I am trying to learn low-level programming and always wanted to create myself an OS, so this is perfect :D.

