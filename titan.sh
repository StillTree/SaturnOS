#!/bin/bash

QEMU_BINARY="qemu-system-x86_64"
RELEASE=false
BUILD_KERNEL=true
BUILD_BOOTLOADER=true
ENABLE_KVM=""

function PrintUsage()
{
	echo "SaturnOS's build script."
	echo
	echo "Usage: $0 [command] [options]"
	echo
	echo "Commands:"
	echo "	build, b    Compiles the entire operating system"
	echo "	run, r      Runs the operating system using QEMU"
	echo
	echo "Options:"
	echo "	-r, --release           Enables optimizations and doesn't include debug symbols"
	echo "	--only-kernel           Invokes the Makefile only in the kernel's folder"
	echo "	--only-bootloader       Invokes the Makefile only in the bootloader's folder"
	echo "	--with-qemu <binary>    Uses the given binary instead of the default qemu-system-x86_64"
	echo "	--enable-kvm            Enables KVM in QEMU"

}

if [[ $# -le 0 ]]; then
	PrintUsage
	exit 0
fi

while [[ $# -gt 0 ]]; do
	case $1 in
		# So I don't actually check for the build option, because it is the only other option
		run|r)
			RUN=true
			shift
			;;
		-r|--release)
			RELEASE=true
			shift
			;;
		--only-kernel)
			BUILD_BOOTLOADER=false
			shift
			;;
		--only-bootloader)
			BUILD_KERNEL=false
			shift
			;;
		--with-qemu)
			QEMU_BINARY=$2
			shift 2
			;;
		--enable-kvm)
			ENABLE_KVM="-enable-kvm"
			shift
			;;
		*)
			echo "Unknown option: $1"
			PrintUsage
			exit 1
			;;
	esac
done

ARCH=$(uname -m)
if [[ "$OSTYPE" != "linux-gnu"* ]] && [[ "$ARCH" != "x86_64" ]]; then
	echo "Error: KVM unsupported on current OS"
	exit 1
fi

function Run()
{
	echo "Starting QEMU..."
	$QEMU_BINARY $ENABLE_KVM
}
