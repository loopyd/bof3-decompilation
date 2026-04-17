# rebof3 - Breath of Fire III reverse engineering and decompilation

cmake_minimum_required(VERSION 3.20)
project(rebof3 VERSION 0.1.0 LANGUAGES C CXX)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    file(CREATE_LINK
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json"
        SYMBOLIC
    )
endif()

option(BOF3_TARGET_PSX "Build bof3 PSX targets" OFF)
set(BOF3_PSX_PROFILE "capcom97-bof3" CACHE STRING "PSX build profile for bof3")
set_property(CACHE BOF3_PSX_PROFILE PROPERTY STRINGS capcom97-bof3)
set(REBOF3_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(BOF3_PSYQ_ROOT "${REBOF3_ROOT_DIR}/toolchains/psyq/4.7" CACHE PATH "Local PsyQ SDK path")
set(BOF3_PSN00B_SDK_ROOT "${REBOF3_ROOT_DIR}/toolchains/psn00bsdk" CACHE PATH "Local PSn00bSDK root used for PS-X EXE conversion tools")

# bof3 - canonical 1:1 PSX decomp source

# bof3 - PSX targets (MIPS cross-compilation with maspsx + PsyQ)
if(BOF3_TARGET_PSX)
    add_subdirectory(bof3)
endif()
