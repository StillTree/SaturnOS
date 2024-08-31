#!/bin/bash

make all

if [ $? -ne 0 ]; then
	exit
fi

qemu-system-x86_64 \
	-enable-kvm \
	-serial stdio \
	-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE.fd \
	-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_VARS.fd \
	-drive format=raw,file=fat:rw:esp

