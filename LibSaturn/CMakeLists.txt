cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB_RECURSE LIBSATURN_C_FILES CONFIGURE_DEPENDS Source/*.c)
file(GLOB_RECURSE LIBSATURN_ASM_FILES CONFIGURE_DEPENDS Source/*.s)

add_library(LibSaturnAssembly OBJECT ${LIBSATURN_ASM_FILES})
add_library(LibSaturn STATIC ${LIBSATURN_C_FILES})

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	target_compile_definitions(LibSaturn PRIVATE LS_RELEASE)
else()
	target_compile_definitions(LibSaturn PRIVATE LS_DEBUG)
endif()

target_link_libraries(LibSaturn PRIVATE LibSaturnAssembly)

target_include_directories(LibSaturn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_compile_options(LibSaturn PRIVATE
	-mno-mmx -mno-sse -mno-sse2
	-ffreestanding -nostdlib -nostdlibinc
	-Wall -Wextra -Wpedantic
)
