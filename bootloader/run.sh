#!/bin/bash

OUTPUT_DIR="esp/efi/boot"
TARGET_DIR="target/x86_64-unknown-uefi"

if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
fi

if [ "$1" == "--release" ]; then
    BUILD_MODE="--release"
    BUILT_FILE="$TARGET_DIR/release/nutcracker.efi"
else
    BUILD_MODE=""
    BUILT_FILE="$TARGET_DIR/debug/nutcracker.efi"
fi

cargo build $BUILD_MODE --target x86_64-unknown-uefi

if [ $? -ne 0 ]; then
    exit
fi

if [ -f $BUIT_FILE ]; then
    cp "$BUILT_FILE" "$OUTPUT_DIR/bootx64.efi"
else
    echo "Could not copy the bootloader. File not found."
fi

qemu-system-x86_64 \
    -enable-kvm \
    -serial stdio \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE.fd \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_VARS.fd \
    -drive format=raw,file=fat:rw:esp

