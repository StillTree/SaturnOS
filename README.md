# Saturn OS

This is a repository (monorepo?) for storing all necessary components for runnning my own hobby operating system.

> Compiled and tested using GCC, other compilers will most definitely require some minor code changes.

## Structure

```
- Kernel (SaturnKernel)
- Bootloader (Supernova)
- README.md
- LICENSE (GPLv3)
- run.sh (A utility script for running in one command)
- Makefile (calls all the other Makefiles)
...
```

Currently there are 2 main components:
-   The kernel itself - `SaturnKernel`
-   The bootloader - `Supernova`

## Why?

Well, why not? I am trying to learn low-level programming and always wanted to create myself an OS, so this is perfect :D.

