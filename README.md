# Deez OS

This is a repository for storing all necessary components for runnning my own hobby operating system.

I am obviously learning so some shit is just straight up copied and modified from the `bootloader` crate (notably framebuffer logging).
So shoutout to all of these guys for making an amazing pure-Rust bootloader and open sourcing it so others can learn from it
(this obviously applies to all of their crates, amazing work!!!).

(this readme is really as bad as it gets, just straight to the point)

## Structure

```
- kernel (deez_kernel)
- bootloader (nutcracker)
- README.md
...
```

Currently there are 2 main components:
-   The kernel itself - `deez_kernel`
-   The bootloader - `nutcracker` (deez nuts, so obviously I had to name it like that even though it really doesn't make that much sense)

## Why?

Well, why not? I am trying to learn low-level Rust and always wanted to create myself an OS, so this is perfect :D.

