# harness - Breath of Fire III reverse engineering and decompilation

cmake_minimum_required(VERSION 3.21)
project(harness VERSION 0.1.0 LANGUAGES C ASM)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Cache/options are globally visible in a CMake build; keep the project
# namespace so parent projects and dependencies cannot collide with it.
option(HARNESS_TARGET_PSX "Build BOF3 PSX targets" OFF)
set(HARNESS_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
unset(HARNESS_PSN00B_SDK_ROOT CACHE)
set(HARNESS_PSN00B_SDK_ROOT "${HARNESS_ROOT_DIR}/toolchains/psn00bsdk")

if(HARNESS_TARGET_PSX)
    set(HARNESS_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    set(HARNESS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    include("${HARNESS_CMAKE_DIR}/psx.cmake")
    include("${HARNESS_CMAKE_DIR}/artifact.cmake")
    include("${HARNESS_CMAKE_DIR}/target.cmake")

    harness_require_psyq_layout()

    set(HARNESS_EXTRACTED_DIR "out/extracted" CACHE STRING "Root of extracted disc + unpacked EMI files")

    include("${HARNESS_CMAKE_DIR}/sources.cmake")

    add_library(slus_004_22_core STATIC ${HARNESS_CORE_SOURCES})
    harness_apply_target_settings(slus_004_22_core INCLUDE_VISIBILITY PUBLIC)

    harness_find_sdk_tool(HARNESS_ELF2X elf2x)
    set(HARNESS_EXE_LINKER_SCRIPT "${HARNESS_CMAKE_DIR}/toolchains/psx-exe.ld")
    set(HARNESS_PSYQ_RUNTIME_LIBS
        "${HARNESS_PSYQ_LIB_DIR}/libapi.a"
        "${HARNESS_PSYQ_LIB_DIR}/libc.a"
        "${HARNESS_PSYQ_LIB_DIR}/libcard.a"
        "${HARNESS_PSYQ_LIB_DIR}/libcd.a"
        "${HARNESS_PSYQ_LIB_DIR}/libetc.a"
        "${HARNESS_PSYQ_LIB_DIR}/libgpu.a"
        "${HARNESS_PSYQ_LIB_DIR}/libgte.a"
        "${HARNESS_PSYQ_LIB_DIR}/libpad.a"
    )
    foreach(psyq_lib IN LISTS HARNESS_PSYQ_RUNTIME_LIBS)
        if(NOT EXISTS "${psyq_lib}")
            message(FATAL_ERROR "Required PsyQ runtime library not found: ${psyq_lib}")
        endif()
    endforeach()

    add_executable(slus_004_22 ${HARNESS_BOOT_SOURCES})
    set_target_properties(slus_004_22 PROPERTIES
        OUTPUT_NAME "slus_004_22"
        SUFFIX ".elf"
    )
    harness_apply_target_settings(slus_004_22)
    target_link_libraries(slus_004_22 PRIVATE
        slus_004_22_core
        --start-group
        ${HARNESS_PSYQ_RUNTIME_LIBS}
        --end-group
    )
    target_link_options(slus_004_22 PRIVATE
        "-T$<SHELL_PATH:${HARNESS_EXE_LINKER_SCRIPT}>"
    )
    add_custom_command(
        TARGET slus_004_22 POST_BUILD
        COMMAND
            ${HARNESS_ELF2X} -q
            $<SHELL_PATH:$<TARGET_FILE:slus_004_22>>
            $<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/slus_004_22.exe>
        BYPRODUCTS
            ${CMAKE_CURRENT_BINARY_DIR}/slus_004_22.exe
        VERBATIM
    )

    include("${HARNESS_CMAKE_DIR}/targets.cmake")

    get_property(HARNESS_REGISTERED_ARTIFACT_TARGETS GLOBAL PROPERTY HARNESS_ARTIFACT_TARGETS)
    if(HARNESS_REGISTERED_ARTIFACT_TARGETS)
        harness_artifact_write_manifest(HARNESS_ARTIFACT_MANIFEST)
        add_custom_target(artifact_manifest DEPENDS "${HARNESS_ARTIFACT_MANIFEST}")
        add_custom_target(artifacts)
        add_dependencies(artifacts artifact_manifest)
        add_dependencies(artifacts ${HARNESS_REGISTERED_ARTIFACT_TARGETS})
    endif()
endif()
