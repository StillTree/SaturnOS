cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB_RECURSE BOOTLOADER_C_FILES CONFIGURE_DEPENDS Source/*.c)

add_executable(Bootloader ${BOOTLOADER_C_FILES})

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	target_compile_definitions(Bootloader PRIVATE SN_RELEASE)
else()
	target_compile_definitions(Bootloader PRIVATE SN_DEBUG)
endif()

target_include_directories(Bootloader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_compile_options(Bootloader PRIVATE
	-target x86_64-unknown-windows
	-mno-red-zone
	-ffreestanding -nostdlib
	-Wall -Wextra -Wpedantic
)

target_link_options(Bootloader PRIVATE
	-target x86_64-unknown-windows
	-fuse-ld=lld-link
	-Wl,-subsystem:efi_application
	-Wl,-entry:UefiMain
	-nostdlib
)
