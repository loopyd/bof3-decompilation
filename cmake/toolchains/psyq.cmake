cmake_minimum_required(VERSION 3.21)

get_filename_component(REBOF3_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(PSYQ_VERSION "4.7" CACHE STRING "Staged PsyQ SDK version under toolchains/psyq")
set(PSYQ_ROOT "" CACHE PATH "Optional staged PsyQ SDK root override")
set(PSX_C_COMPILER "${REBOF3_ROOT_DIR}/bin/cc" CACHE FILEPATH "PSX C compiler driver")
set(PSX_AS "${REBOF3_ROOT_DIR}/bin/as" CACHE FILEPATH "PSX assembler")
set(PSX_LD "${REBOF3_ROOT_DIR}/bin/ld" CACHE FILEPATH "PSX linker")
unset(BOF3_LOCAL_TOOLCHAIN_BIN CACHE)
unset(BOF3_PSN00B_SDK_ROOT CACHE)
unset(BOF3_PSX_GCC_ROOT CACHE)
unset(BOF3_MASPSX_ASPSX_VERSION CACHE)
unset(BOF3_PSYQ_SDK_KIND CACHE)
unset(BOF3_TOOLCHAIN_NM CACHE)
set(BOF3_LOCAL_TOOLCHAIN_BIN "${REBOF3_ROOT_DIR}/toolchains/psn00b_toolchain/bin")
set(BOF3_PSN00B_SDK_ROOT "${REBOF3_ROOT_DIR}/toolchains/psn00bsdk")
set(BOF3_PSX_GCC_ROOT "${REBOF3_ROOT_DIR}/toolchains/gcc-2.7.2-psx")

if(PSYQ_ROOT)
    get_filename_component(BOF3_ACTIVE_PSYQ_ROOT "${PSYQ_ROOT}" ABSOLUTE)
else()
    set(BOF3_ACTIVE_PSYQ_ROOT "${REBOF3_ROOT_DIR}/toolchains/psyq/${PSYQ_VERSION}")
endif()

set(BOF3_PSYQ_SDK_KIND "original")
set(BOF3_MASPSX_ASPSX_VERSION "2.56")
if(NOT EXISTS "${BOF3_ACTIVE_PSYQ_ROOT}/include/libgpu.h")
    message(FATAL_ERROR
        "PsyQ ${PSYQ_VERSION} headers not found at ${BOF3_ACTIVE_PSYQ_ROOT}. "
        "Run 'just psyq' to stage the configured SDK.")
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR mips)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
    PSYQ_VERSION
    PSYQ_ROOT
    PSX_C_COMPILER
    PSX_AS
    PSX_LD)

if(NOT EXISTS "${PSX_C_COMPILER}")
    message(FATAL_ERROR "PSX C compiler driver not found at ${PSX_C_COMPILER}")
endif()
if(NOT EXISTS "${BOF3_PSX_GCC_ROOT}/gcc")
    message(FATAL_ERROR
        "Canonical gcc-2.7.2-psx compiler not found at ${BOF3_PSX_GCC_ROOT}. "
        "Run 'just setup' first.")
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
    ERROR_MESSAGE "missing repo-local PSX assembler at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-as; run 'just setup'")
bof3_find_program(MIPSEL_LD
    NAMES mipsel-none-elf-ld
    ERROR_MESSAGE "missing repo-local PSX linker at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-ld; run 'just setup'")
bof3_find_program(MIPSEL_AR
    NAMES mipsel-none-elf-ar
    ERROR_MESSAGE "missing repo-local PSX archiver at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-ar; run 'just setup'")
bof3_find_program(MIPSEL_RANLIB
    NAMES mipsel-none-elf-ranlib
    ERROR_MESSAGE "missing repo-local PSX ranlib at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-ranlib; run 'just setup'")
bof3_find_program(MIPSEL_OBJCOPY
    NAMES mipsel-none-elf-objcopy
    ERROR_MESSAGE "missing repo-local PSX objcopy at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-objcopy; run 'just setup'")
bof3_find_program(MIPSEL_OBJDUMP
    NAMES mipsel-none-elf-objdump
    ERROR_MESSAGE "missing repo-local PSX objdump at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-objdump; run 'just setup'")
bof3_find_program(MIPSEL_NM
    NAMES mipsel-none-elf-nm
    ERROR_MESSAGE "missing repo-local PSX nm at ${BOF3_LOCAL_TOOLCHAIN_BIN}/mipsel-none-elf-nm; run 'just setup'")

set(CMAKE_C_COMPILER "${PSX_C_COMPILER}")
set(CMAKE_ASM_COMPILER "${PSX_AS}")
set(CMAKE_LINKER "${PSX_LD}")
set(CMAKE_AR "${REBOF3_ROOT_DIR}/bin/ar")
set(CMAKE_RANLIB "${REBOF3_ROOT_DIR}/bin/ranlib")
set(CMAKE_OBJCOPY "${REBOF3_ROOT_DIR}/bin/objcopy")
set(CMAKE_OBJDUMP "${REBOF3_ROOT_DIR}/bin/objdump")
set(BOF3_TOOLCHAIN_NM "${REBOF3_ROOT_DIR}/bin/nm")

set(CMAKE_C_FLAGS_INIT "-O2 -G0 -funsigned-char -msoft-float -gcoff -Wa,--aspsx-version=${BOF3_MASPSX_ASPSX_VERSION} -Wa,-G0,-EL,-mips1")
set(CMAKE_ASM_FLAGS_INIT "-G0 -EL -mips1")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-EL -static -L${BOF3_ACTIVE_PSYQ_ROOT}/lib")
set(CMAKE_ASM_COMPILE_OBJECT
    "${PSX_AS} <FLAGS> <INCLUDES> -o <OBJECT> <SOURCE>")
set(CMAKE_C_LINK_EXECUTABLE
    "${PSX_LD} <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_LINK_EXECUTABLE
    "${PSX_LD} <CMAKE_ASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_FIND_ROOT_PATH "${BOF3_ACTIVE_PSYQ_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(BOF3_PSYQ_INCLUDE_DIR "${BOF3_ACTIVE_PSYQ_ROOT}/include")
set(BOF3_PSYQ_LIB_DIR "${BOF3_ACTIVE_PSYQ_ROOT}/lib")
