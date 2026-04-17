cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED BOF3_PSX_PROFILE)
    set(BOF3_PSX_PROFILE "capcom97-bof3")
endif()
if(NOT DEFINED BOF3_PSN00B_SDK_ROOT)
    set(BOF3_PSN00B_SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../toolchains/psn00bsdk")
endif()
if(NOT DEFINED BOF3_PSX_GCC_ROOT)
    set(BOF3_PSX_GCC_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../toolchains/gcc-2.7.2-psx")
endif()

set(BOF3_PSYQ_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../toolchains/psyq/4.7"
    CACHE PATH "Active PsyQ SDK root" FORCE)
set(BOF3_LOCAL_TOOLCHAIN_BIN "${CMAKE_CURRENT_LIST_DIR}/../../toolchains/psn00b_toolchain/bin"
    CACHE PATH "Repo-local GNU binutils toolchain bin dir" FORCE)
set(BOF3_PSN00B_SDK_ROOT "${BOF3_PSN00B_SDK_ROOT}"
    CACHE PATH "Repo-local PSn00bSDK root" FORCE)
set(BOF3_PSX_GCC_ROOT "${BOF3_PSX_GCC_ROOT}"
    CACHE PATH "Repo-local canonical BOF3 gcc-2.7.2-psx root" FORCE)

if(BOF3_PSX_PROFILE STREQUAL "capcom97-bof3")
    set(BOF3_PSYQ_SDK_KIND "original" CACHE STRING "Active PsyQ SDK kind" FORCE)
    set(BOF3_MASPSX_ASPSX_VERSION "2.56" CACHE STRING "ASPSX behavior version for maspsx" FORCE)
    if(NOT EXISTS "${BOF3_PSYQ_ROOT}/include/libgpu.h")
        message(FATAL_ERROR
            "PsyQ 4.7 headers not found at ${BOF3_PSYQ_ROOT}. "
            "Run 'make setup-psyq PSYQ_ARCHIVE=inputs/psyq-4.7-converted-full.7z' first.")
    endif()
else()
    message(FATAL_ERROR "Unsupported BOF3_PSX_PROFILE='${BOF3_PSX_PROFILE}'.")
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR mips)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
    BOF3_PSX_PROFILE
    BOF3_PSYQ_ROOT
    BOF3_LOCAL_TOOLCHAIN_BIN
    BOF3_PSX_GCC_ROOT
    BOF3_PSN00B_SDK_ROOT)

set(MASPSX_CC "${CMAKE_CURRENT_LIST_DIR}/../../bin/maspsx-cc")
if(NOT EXISTS ${MASPSX_CC})
    message(FATAL_ERROR "maspsx-cc wrapper not found at ${MASPSX_CC}")
endif()
if(NOT EXISTS "${BOF3_PSX_GCC_ROOT}/gcc")
    message(FATAL_ERROR
        "Canonical gcc-2.7.2-psx compiler not found at ${BOF3_PSX_GCC_ROOT}. "
        "Run 'make setup-open' first.")
endif()

function(bof3_find_program out_var)
    cmake_parse_arguments(ARG "" "ERROR_MESSAGE" "NAMES" ${ARGN})
    if(EXISTS "${BOF3_LOCAL_TOOLCHAIN_BIN}")
        find_program(${out_var} NAMES ${ARG_NAMES} PATHS "${BOF3_LOCAL_TOOLCHAIN_BIN}" NO_DEFAULT_PATH)
    endif()
    if(NOT ${out_var})
        find_program(${out_var} NAMES ${ARG_NAMES})
    endif()
    if(NOT ${out_var})
        message(FATAL_ERROR "${ARG_ERROR_MESSAGE}")
    endif()
endfunction()

bof3_find_program(MIPSEL_GCC
    NAMES mipsel-none-elf-gcc mipsel-linux-gnu-gcc
    ERROR_MESSAGE "missing PSX C compiler driver (expected repo-local psn00b toolchain or system mipsel cross gcc)")
bof3_find_program(MIPSEL_GXX
    NAMES mipsel-none-elf-g++ mipsel-linux-gnu-g++
    ERROR_MESSAGE "missing PSX C++ compiler (expected repo-local psn00b toolchain or system mipsel cross g++)")
bof3_find_program(MIPSEL_AS
    NAMES mipsel-none-elf-as mipsel-linux-gnu-as
    ERROR_MESSAGE "missing PSX assembler (expected repo-local psn00b toolchain or system mipsel cross assembler)")
bof3_find_program(MIPSEL_LD
    NAMES mipsel-none-elf-ld mipsel-linux-gnu-ld
    ERROR_MESSAGE "missing PSX linker (expected repo-local psn00b toolchain or system mipsel cross linker)")
bof3_find_program(MIPSEL_AR
    NAMES mipsel-none-elf-ar mipsel-linux-gnu-ar
    ERROR_MESSAGE "missing PSX archiver (expected repo-local psn00b toolchain or system mipsel cross ar)")
bof3_find_program(MIPSEL_RANLIB
    NAMES mipsel-none-elf-ranlib mipsel-linux-gnu-ranlib
    ERROR_MESSAGE "missing PSX ranlib (expected repo-local psn00b toolchain or system mipsel cross ranlib)")
bof3_find_program(MIPSEL_OBJCOPY
    NAMES mipsel-none-elf-objcopy mipsel-linux-gnu-objcopy
    ERROR_MESSAGE "missing PSX objcopy (expected repo-local psn00b toolchain or system mipsel cross objcopy)")
bof3_find_program(MIPSEL_OBJDUMP
    NAMES mipsel-none-elf-objdump mipsel-linux-gnu-objdump
    ERROR_MESSAGE "missing PSX objdump (expected repo-local psn00b toolchain or system mipsel cross objdump)")
bof3_find_program(MIPSEL_NM
    NAMES mipsel-none-elf-nm mipsel-linux-gnu-nm
    ERROR_MESSAGE "missing PSX nm (expected repo-local psn00b toolchain or system mipsel cross nm)")

set(CMAKE_C_COMPILER "${MASPSX_CC}")
set(CMAKE_CXX_COMPILER ${MIPSEL_GXX})
set(CMAKE_ASM_COMPILER ${MIPSEL_GCC})
set(CMAKE_LINKER ${MIPSEL_LD})
set(CMAKE_AR ${MIPSEL_AR})
set(CMAKE_RANLIB ${MIPSEL_RANLIB})
set(CMAKE_OBJCOPY ${MIPSEL_OBJCOPY})
set(CMAKE_OBJDUMP ${MIPSEL_OBJDUMP})
set(BOF3_TOOLCHAIN_NM ${MIPSEL_NM}
    CACHE FILEPATH "PSX nm used for symbol maps and diagnostics" FORCE)

set(CMAKE_C_FLAGS_INIT "--profile=${BOF3_PSX_PROFILE}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-nostdlib -static -L${BOF3_PSYQ_ROOT}/lib")
set(CMAKE_C_LINK_EXECUTABLE
    "${MIPSEL_GCC} <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE
    "${MIPSEL_GXX} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_LINK_EXECUTABLE
    "${MIPSEL_GCC} <FLAGS> <CMAKE_ASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_FIND_ROOT_PATH "${BOF3_PSYQ_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(BOF3_PSYQ_INCLUDE_DIR "${BOF3_PSYQ_ROOT}/include")
set(BOF3_PSYQ_LIB_DIR "${BOF3_PSYQ_ROOT}/lib")
