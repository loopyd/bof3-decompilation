cmake_minimum_required(VERSION 3.21)

get_filename_component(REBOF3_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(BOF3_PSX_PROFILE "capcom97-bof3" CACHE STRING "PSX build profile for bof3")
set_property(CACHE BOF3_PSX_PROFILE PROPERTY STRINGS capcom97-bof3)
set(BOF3_PSYQ_VERSION "4.7" CACHE STRING "Staged PsyQ SDK version under toolchains/psyq")
set(BOF3_PSYQ_ROOT "" CACHE PATH "Optional staged PsyQ SDK root override")
unset(BOF3_LOCAL_TOOLCHAIN_BIN CACHE)
unset(BOF3_PSN00B_SDK_ROOT CACHE)
unset(BOF3_PSX_GCC_ROOT CACHE)
unset(BOF3_MASPSX_ASPSX_VERSION CACHE)
unset(BOF3_PSYQ_SDK_KIND CACHE)
unset(BOF3_TOOLCHAIN_NM CACHE)
set(BOF3_LOCAL_TOOLCHAIN_BIN "${REBOF3_ROOT_DIR}/toolchains/psn00b_toolchain/bin")
set(BOF3_PSN00B_SDK_ROOT "${REBOF3_ROOT_DIR}/toolchains/psn00bsdk")
set(BOF3_PSX_GCC_ROOT "${REBOF3_ROOT_DIR}/toolchains/gcc-2.7.2-psx")

if(BOF3_PSYQ_ROOT)
    get_filename_component(BOF3_ACTIVE_PSYQ_ROOT "${BOF3_PSYQ_ROOT}" ABSOLUTE)
else()
    set(BOF3_ACTIVE_PSYQ_ROOT "${REBOF3_ROOT_DIR}/toolchains/psyq/${BOF3_PSYQ_VERSION}")
endif()

if(BOF3_PSX_PROFILE STREQUAL "capcom97-bof3")
    set(BOF3_PSYQ_SDK_KIND "original")
    set(BOF3_MASPSX_ASPSX_VERSION "2.56")
    if(NOT EXISTS "${BOF3_ACTIVE_PSYQ_ROOT}/include/libgpu.h")
        message(FATAL_ERROR
            "PsyQ ${BOF3_PSYQ_VERSION} headers not found at ${BOF3_ACTIVE_PSYQ_ROOT}. "
            "Run 'bin/download-psyq' for the default SDK, or "
            "'bin/setup-psyq --version ${BOF3_PSYQ_VERSION} --archive <path>'.")
    endif()
else()
    message(FATAL_ERROR "Unsupported BOF3_PSX_PROFILE='${BOF3_PSX_PROFILE}'.")
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR mips)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
    BOF3_PSX_PROFILE
    BOF3_PSYQ_VERSION
    BOF3_PSYQ_ROOT)

set(MASPSX_CC "${REBOF3_ROOT_DIR}/bin/maspsx-cc")
if(NOT EXISTS "${MASPSX_CC}")
    message(FATAL_ERROR "maspsx-cc wrapper not found at ${MASPSX_CC}")
endif()
if(NOT EXISTS "${BOF3_PSX_GCC_ROOT}/gcc")
    message(FATAL_ERROR
        "Canonical gcc-2.7.2-psx compiler not found at ${BOF3_PSX_GCC_ROOT}. "
        "Run 'bin/setup-psx-toolchain' or 'bin/setup-open' first.")
endif()

function(bof3_find_program out_var)
    cmake_parse_arguments(ARG "" "ERROR_MESSAGE" "NAMES" ${ARGN})
    find_program(${out_var} NAMES ${ARG_NAMES} PATHS "${BOF3_LOCAL_TOOLCHAIN_BIN}" NO_DEFAULT_PATH)
    if(NOT ${out_var})
        message(FATAL_ERROR "${ARG_ERROR_MESSAGE}")
    endif()
endfunction()

bof3_find_program(MIPSEL_AS
    NAMES mipsel-none-elf-as
    ERROR_MESSAGE "missing repo-local PSX assembler at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-as; run 'bin/setup-psx-toolchain'")
bof3_find_program(MIPSEL_LD
    NAMES mipsel-none-elf-ld
    ERROR_MESSAGE "missing repo-local PSX linker at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-ld; run 'bin/setup-psx-toolchain'")
bof3_find_program(MIPSEL_AR
    NAMES mipsel-none-elf-ar
    ERROR_MESSAGE "missing repo-local PSX archiver at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-ar; run 'bin/setup-psx-toolchain'")
bof3_find_program(MIPSEL_RANLIB
    NAMES mipsel-none-elf-ranlib
    ERROR_MESSAGE "missing repo-local PSX ranlib at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-ranlib; run 'bin/setup-psx-toolchain'")
bof3_find_program(MIPSEL_OBJCOPY
    NAMES mipsel-none-elf-objcopy
    ERROR_MESSAGE "missing repo-local PSX objcopy at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-objcopy; run 'bin/setup-psx-toolchain'")
bof3_find_program(MIPSEL_OBJDUMP
    NAMES mipsel-none-elf-objdump
    ERROR_MESSAGE "missing repo-local PSX objdump at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-objdump; run 'bin/setup-psx-toolchain'")
bof3_find_program(MIPSEL_NM
    NAMES mipsel-none-elf-nm
    ERROR_MESSAGE "missing repo-local PSX nm at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-nm; run 'bin/setup-psx-toolchain'")

set(CMAKE_C_COMPILER "${MASPSX_CC}")
set(CMAKE_ASM_COMPILER ${MIPSEL_AS})
set(CMAKE_LINKER ${MIPSEL_LD})
set(CMAKE_AR ${MIPSEL_AR})
set(CMAKE_RANLIB ${MIPSEL_RANLIB})
set(CMAKE_OBJCOPY ${MIPSEL_OBJCOPY})
set(CMAKE_OBJDUMP ${MIPSEL_OBJDUMP})
set(BOF3_TOOLCHAIN_NM ${MIPSEL_NM})

set(CMAKE_C_FLAGS_INIT "--profile=${BOF3_PSX_PROFILE} --psyq-root=${BOF3_ACTIVE_PSYQ_ROOT} -Wa,--aspsx-version=${BOF3_MASPSX_ASPSX_VERSION}")
set(CMAKE_ASM_FLAGS_INIT "-G0 -EL -mips1")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-EL -static -L${BOF3_ACTIVE_PSYQ_ROOT}/lib")
set(CMAKE_ASM_COMPILE_OBJECT
    "${MIPSEL_AS} <FLAGS> <INCLUDES> -o <OBJECT> <SOURCE>")
set(CMAKE_C_LINK_EXECUTABLE
    "${MIPSEL_LD} <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_LINK_EXECUTABLE
    "${MIPSEL_LD} <CMAKE_ASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_FIND_ROOT_PATH "${BOF3_ACTIVE_PSYQ_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(BOF3_PSYQ_INCLUDE_DIR "${BOF3_ACTIVE_PSYQ_ROOT}/include")
set(BOF3_PSYQ_LIB_DIR "${BOF3_ACTIVE_PSYQ_ROOT}/lib")
