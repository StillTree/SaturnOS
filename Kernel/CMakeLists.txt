cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 23)

file(GLOB_RECURSE KERNEL_C_FILES CONFIGURE_DEPENDS Source/*.c)
file(GLOB_RECURSE KERNEL_ASM_FILES CONFIGURE_DEPENDS Source/*.s)

add_library(KernelAssembly OBJECT ${KERNEL_ASM_FILES})
add_executable(Kernel ${KERNEL_C_FILES})

set_source_files_properties(Source/InterruptHandlers.c PROPERTIES COMPILE_FLAGS -mgeneral-regs-only)

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	target_compile_definitions(Kernel PRIVATE SK_RELEASE)
else()
	target_compile_definitions(Kernel PRIVATE SK_DEBUG)
endif()

target_include_directories(Kernel PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Library/Include
)

target_compile_options(Kernel PRIVATE
	-mno-red-zone
	-mcmodel=kernel
	-mno-mmx -mno-sse -mno-sse2
	-ffreestanding -nostdlib -nostdlibinc
	-Wall -Wextra -Wpedantic
)

target_link_options(Kernel PRIVATE
	-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
	-static -Wl,-z,max-page-size=4096
	-nostdlib
)

target_link_libraries(Kernel PRIVATE
	KernelAssembly
	${SYSROOT_LIB_KERNEL}/libCompilerRT.a
)

add_dependencies(Kernel SystemRoot)
