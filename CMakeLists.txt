cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE ToolChain/x86_64_saturnos-toolchain.cmake)

project(SaturnOS VERSION 0.0.0.1 LANGUAGES C ASM)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(QEMU_BIN "qemu-system-x86_64" CACHE STRING "QEMU binary to run SaturnOS")
set(QEMU_OVMF_CODE "/opt/OVMF/OVMF_CODE.fd" CACHE STRING "Path to OVMF_CODE.fd")
set(QEMU_OVMF_VARS "/opt/OVMF/OVMF_VARS.fd" CACHE STRING "Path to OVMF_VARS.fd")
option(QEMU_ENABLE_KVM "Enable KVM acceleration" OFF)
option(QEMU_REMOVE_GDB "Enable remove GDB debugging" OFF)

set(SYSROOT_DIR ${CMAKE_BINARY_DIR}/SystemRoot)
set(SYSROOT_LIB_USER ${SYSROOT_DIR}/Library)
set(SYSROOT_LIB_KERNEL ${SYSROOT_DIR}/Library/Kernel)
set(SYSROOT_INCLUDE ${SYSROOT_DIR}/Include)

set(COMPILER_RT_USER ${CMAKE_SOURCE_DIR}/ToolChain/Build/CompilerRTUser/lib/linux/libclang_rt.builtins-x86_64.a
	CACHE FILEPATH "Path to the compiled, userspace, CompilerRT library")
set(COMPILER_RT_KERNEL ${CMAKE_SOURCE_DIR}/ToolChain/Build/CompilerRTKernel/lib/linux/libclang_rt.builtins-x86_64.a
	CACHE FILEPATH "Path to the compiled, kernel-ready, CompilerRT library")

set(ESP_DIR ${CMAKE_BINARY_DIR}/ESP)
set(ESP_BOOT ${ESP_DIR}/EFI/Boot)
set(ESP_SUPERNOVA ${ESP_DIR}/Supernova)

add_subdirectory(LibSaturn)
add_subdirectory(Kernel)
add_subdirectory(Bootloader)
add_subdirectory(SaturnCRT)

add_custom_target(SystemRoot
	COMMAND ${CMAKE_COMMAND} -E make_directory ${SYSROOT_LIB_KERNEL}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${SYSROOT_LIB_USER}
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/Assets/SystemRoot
		${SYSROOT_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:LibSaturn>
		${SYSROOT_LIB_USER}/libSaturn.a
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:SaturnCRT>
		${SYSROOT_LIB_USER}/libSaturnCRT.a
	COMMAND ${CMAKE_COMMAND} -E copy
		${COMPILER_RT_USER}
		${SYSROOT_LIB_USER}/libCompilerRT.a
	COMMAND ${CMAKE_COMMAND} -E copy
		${COMPILER_RT_KERNEL}
		${SYSROOT_LIB_KERNEL}/libCompilerRT.a
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/LibSaturn/Include
		${SYSROOT_INCLUDE}
	COMMENT "Creating SystemRoot with LibSaturn"
	DEPENDS LibSaturn SaturnCRT
)

add_custom_target(ESP
	COMMAND ${CMAKE_COMMAND} -E make_directory ${ESP_BOOT}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${ESP_SUPERNOVA}
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/Assets/ESP
		${ESP_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:Bootloader>
		${ESP_BOOT}/BootX64.efi
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:Kernel>
		${ESP_SUPERNOVA}/Kernel.elf
	COMMENT "Preparing ESP folders with the bootloader and the kernel"
	DEPENDS Kernel Bootloader
)

add_custom_target(Run
	COMMAND ${QEMU_BIN}
		-drive if=pflash,format=raw,readonly=on,file=${QEMU_OVMF_CODE}
		-drive if=pflash,format=raw,readonly=on,file=${QEMU_OVMF_VARS}
		-machine q35
		-serial stdio
		-drive format=raw,file=fat:rw:${ESP_DIR}
		-drive format=raw,if=none,id=sata,file=${CMAKE_SOURCE_DIR}/Sata.img
		-device ahci,id=ahci
		-device ide-hd,drive=sata,bus=ahci.0
		$<$<BOOL:${QEMU_REMOTE_GDB}>:-s>
		$<$<BOOL:${QEMU_REMOTE_GDB}>:-S>
		$<$<BOOL:${QEMU_ENABLE_KVM}>:-enable-kvm>
		DEPENDS SystemRoot ESP
	USES_TERMINAL
	COMMENT "Running SaturnOS in QEMU"
)
