cmake_minimum_required(VERSION 3.21)

get_filename_component(HARNESS_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(HARNESS_PROFILE "native/capcom97" CACHE STRING "Named harness compiler/toolchain profile")
set(HARNESS_PSYQ_ROOT "" CACHE PATH "Optional staged PsyQ SDK root override")
set(PSX_C_COMPILER "${HARNESS_ROOT_DIR}/bin/cc" CACHE FILEPATH "PSX C compiler driver")
set(PSX_AS "${HARNESS_ROOT_DIR}/bin/as" CACHE FILEPATH "PSX assembler")
set(PSX_LD "${HARNESS_ROOT_DIR}/bin/ld" CACHE FILEPATH "PSX linker")
unset(HARNESS_LOCAL_TOOLCHAIN_BIN CACHE)
unset(HARNESS_PSN00B_SDK_ROOT CACHE)
unset(HARNESS_PSX_GCC_ROOT CACHE)
unset(HARNESS_MASPSX_ASPSX_VERSION CACHE)
unset(HARNESS_PSYQ_SDK_KIND CACHE)
unset(HARNESS_TOOLCHAIN_NM CACHE)
set(HARNESS_LOCAL_TOOLCHAIN_BIN "${HARNESS_ROOT_DIR}/toolchains/psn00b_toolchain/bin")
set(HARNESS_PSN00B_SDK_ROOT "${HARNESS_ROOT_DIR}/toolchains/psn00bsdk")
set(HARNESS_PSX_GCC_ROOT "${HARNESS_ROOT_DIR}/toolchains/gcc-2.7.2-psx")

if(HARNESS_PROFILE STREQUAL "original/psyq36")
    set(HARNESS_PSYQ_SDK_VERSION "3.6")
elseif(HARNESS_PROFILE STREQUAL "original/psyq40")
    set(HARNESS_PSYQ_SDK_VERSION "4.0")
elseif(HARNESS_PROFILE STREQUAL "native/capcom97")
    set(HARNESS_PSYQ_SDK_VERSION "4.7")
else()
    message(FATAL_ERROR "Unknown HARNESS_PROFILE: ${HARNESS_PROFILE}")
endif()
if(HARNESS_PSYQ_ROOT)
    get_filename_component(HARNESS_ACTIVE_PSYQ_ROOT "${HARNESS_PSYQ_ROOT}" ABSOLUTE)
else()
    set(HARNESS_ACTIVE_PSYQ_ROOT "${HARNESS_ROOT_DIR}/toolchains/psyq/${HARNESS_PSYQ_SDK_VERSION}")
endif()

set(HARNESS_PSYQ_SDK_KIND "original")
set(HARNESS_MASPSX_ASPSX_VERSION "2.56")
if(NOT EXISTS "${HARNESS_ACTIVE_PSYQ_ROOT}/include/libgpu.h")
    message(FATAL_ERROR
        "PsyQ ${HARNESS_PSYQ_SDK_VERSION} headers not found at ${HARNESS_ACTIVE_PSYQ_ROOT}. "
        "Run 'just psyq' to stage the configured SDK.")
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR mips)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
    HARNESS_PROFILE
    HARNESS_PSYQ_ROOT
    PSX_C_COMPILER
    PSX_AS
    PSX_LD)

if(NOT EXISTS "${PSX_C_COMPILER}")
    message(FATAL_ERROR "PSX C compiler driver not found at ${PSX_C_COMPILER}")
endif()
if(NOT EXISTS "${HARNESS_PSX_GCC_ROOT}/gcc")
    message(FATAL_ERROR
        "Canonical gcc-2.7.2-psx compiler not found at ${HARNESS_PSX_GCC_ROOT}. "
        "Run 'just setup' first.")
endif()

function(harness_find_program out_var)
    cmake_parse_arguments(ARG "" "ERROR_MESSAGE" "NAMES" ${ARGN})
    find_program(${out_var} NAMES ${ARG_NAMES} PATHS "${HARNESS_LOCAL_TOOLCHAIN_BIN}" NO_DEFAULT_PATH)
    if(NOT ${out_var})
        message(FATAL_ERROR "${ARG_ERROR_MESSAGE}")
    endif()
endfunction()

harness_find_program(MIPSEL_AS
    NAMES mipsel-none-elf-as
    ERROR_MESSAGE "missing repo-local PSX assembler at ${HARNESS_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-as; run 'just setup'")
harness_find_program(MIPSEL_LD
    NAMES mipsel-none-elf-ld
    ERROR_MESSAGE "missing repo-local PSX linker at ${HARNESS_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-ld; run 'just setup'")
harness_find_program(MIPSEL_AR
    NAMES mipsel-none-elf-ar
    ERROR_MESSAGE "missing repo-local PSX archiver at ${HARNESS_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-ar; run 'just setup'")
harness_find_program(MIPSEL_RANLIB
    NAMES mipsel-none-elf-ranlib
    ERROR_MESSAGE "missing repo-local PSX ranlib at ${HARNESS_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-ranlib; run 'just setup'")
harness_find_program(MIPSEL_OBJCOPY
    NAMES mipsel-none-elf-objcopy
    ERROR_MESSAGE "missing repo-local PSX objcopy at ${HARNESS_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-objcopy; run 'just setup'")
harness_find_program(MIPSEL_OBJDUMP
    NAMES mipsel-none-elf-objdump
    ERROR_MESSAGE "missing repo-local PSX objdump at ${HARNESS_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-objdump; run 'just setup'")
harness_find_program(MIPSEL_NM
    NAMES mipsel-none-elf-nm
    ERROR_MESSAGE "missing repo-local PSX nm at ${HARNESS_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-nm; run 'just setup'")

set(CMAKE_C_COMPILER "${PSX_C_COMPILER}")
set(CMAKE_ASM_COMPILER "${PSX_AS}")
set(CMAKE_LINKER "${PSX_LD}")
set(CMAKE_AR "${HARNESS_ROOT_DIR}/bin/ar")
set(CMAKE_RANLIB "${HARNESS_ROOT_DIR}/bin/ranlib")
set(CMAKE_OBJCOPY "${HARNESS_ROOT_DIR}/bin/objcopy")
set(CMAKE_OBJDUMP "${HARNESS_ROOT_DIR}/bin/objdump")
set(HARNESS_TOOLCHAIN_NM "${HARNESS_ROOT_DIR}/bin/nm")

set(CMAKE_C_FLAGS_INIT "-O2 -G0 -funsigned-char -msoft-float -gcoff -Wa,--aspsx-version=${HARNESS_MASPSX_ASPSX_VERSION} -Wa,-G0,-EL,-mips1")
set(CMAKE_ASM_FLAGS_INIT "-G0 -EL -mips1")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-EL -static -L${HARNESS_ACTIVE_PSYQ_ROOT}/lib")
set(CMAKE_ASM_COMPILE_OBJECT
    "${PSX_AS} <FLAGS> <INCLUDES> -o <OBJECT> <SOURCE>")
set(CMAKE_C_LINK_EXECUTABLE
    "${PSX_LD} <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_LINK_EXECUTABLE
    "${PSX_LD} <CMAKE_ASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_FIND_ROOT_PATH "${HARNESS_ACTIVE_PSYQ_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(HARNESS_PSYQ_INCLUDE_DIR "${HARNESS_ACTIVE_PSYQ_ROOT}/include")
set(HARNESS_PSYQ_LIB_DIR "${HARNESS_ACTIVE_PSYQ_ROOT}/lib")
