cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB_RECURSE SATURNCRT_C_FILES CONFIGURE_DEPENDS Source/*.c)

add_library(SaturnCRT STATIC ${SATURNCRT_C_FILES})

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	target_compile_definitions(SaturnCRT PRIVATE SC_RELEASE)
else()
	target_compile_definitions(SaturnCRT PRIVATE SC_DEBUG)
endif()

target_include_directories(SaturnCRT PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_compile_options(SaturnCRT PRIVATE
	-mno-mmx -mno-sse -mno-sse2
	-ffreestanding -nostdlib -nostdlibinc
	-Wall -Wextra -Wpedantic
)
