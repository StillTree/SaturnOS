#!/bin/bash

make all

if [ $? -ne 0 ]; then
	exit
fi

ENABLE_KVM="-enable-kvm"
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
	ENABLE_KVM=""
fi

if [ -z "${OVMF_CODE_LOCATION+x}" ]; then
	OVMF_CODE_LOCATION="/opt/OVMF/OVMF_CODE.fd"
fi

if [ -z "${OVMF_VARS_LOCATION+x}" ]; then
	OVMF_VARS_LOCATION="/opt/OVMF/OVMF_VARS.fd"
fi

qemu-system-x86_64 \
	$ENABLE_KVM \
	-serial stdio \
	-drive if=pflash,format=raw,readonly=on,file=$OVMF_CODE_LOCATION \
	-drive if=pflash,format=raw,readonly=on,file=$OVMF_VARS_LOCATION \
	-drive format=raw,file=fat:rw:esp

