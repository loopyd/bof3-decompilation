cmake_minimum_required(VERSION 3.21)

project(bof3 LANGUAGES C ASM)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(DEFINED REBOF3_ROOT_DIR)
    set(BOF3_REPO_ROOT_DIR "${REBOF3_ROOT_DIR}")
else()
    get_filename_component(BOF3_REPO_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

set(BOF3_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(BOF3_CMAKE_DIR "${BOF3_PROJECT_DIR}/cmake")
set(BOF3_SHARED_CMAKE_DIR "${BOF3_REPO_ROOT_DIR}/cmake")
set(BOF3_INCLUDE_DIR "${BOF3_PROJECT_DIR}/include")

find_package(Python3 REQUIRED COMPONENTS Interpreter)

include(${BOF3_SHARED_CMAKE_DIR}/bof3_sdk_helpers.cmake)
include(${BOF3_SHARED_CMAKE_DIR}/bof3_target_helpers.cmake)
include(${BOF3_SHARED_CMAKE_DIR}/bof3_artifact_registry.cmake)
include(${BOF3_SHARED_CMAKE_DIR}/bof3_module_helpers.cmake)

bof3_require_psx_profile("capcom97-bof3")
bof3_require_psyq_layout()

set(BOF3_EXTRACTED_DIR "output/extracted" CACHE STRING "Root of extracted disc + unpacked EMI files")

include(${BOF3_CMAKE_DIR}/sources.cmake)

add_library(bof3 STATIC ${BOF3_SOURCES})
bof3_apply_common_target_settings(bof3 INCLUDE_VISIBILITY PUBLIC)

bof3_find_sdk_tool(BOF3_ELF2X elf2x)
set(BOF3_EXE_LINKER_SCRIPT "${BOF3_SHARED_CMAKE_DIR}/toolchains/psx-exe.ld")
set(BOF3_PSYQ_RUNTIME_LIBS
    "${BOF3_PSYQ_LIB_DIR}/libapi.a"
    "${BOF3_PSYQ_LIB_DIR}/libc.a"
    "${BOF3_PSYQ_LIB_DIR}/libcard.a"
    "${BOF3_PSYQ_LIB_DIR}/libcd.a"
    "${BOF3_PSYQ_LIB_DIR}/libetc.a"
    "${BOF3_PSYQ_LIB_DIR}/libgpu.a"
    "${BOF3_PSYQ_LIB_DIR}/libgte.a"
    "${BOF3_PSYQ_LIB_DIR}/libpad.a"
)
foreach(psyq_lib IN LISTS BOF3_PSYQ_RUNTIME_LIBS)
    if(NOT EXISTS "${psyq_lib}")
        message(FATAL_ERROR "Required PsyQ runtime library not found: ${psyq_lib}")
    endif()
endforeach()

add_executable(bof3_exe ${BOF3_BOOT_SOURCES})
set_target_properties(bof3_exe PROPERTIES
    OUTPUT_NAME "bof3"
    SUFFIX ".elf"
)
bof3_apply_common_target_settings(bof3_exe)
target_link_libraries(bof3_exe PRIVATE
    bof3
    --start-group
    ${BOF3_PSYQ_RUNTIME_LIBS}
    --end-group
)
target_link_options(bof3_exe PRIVATE
    "-T$<SHELL_PATH:${BOF3_EXE_LINKER_SCRIPT}>"
)
add_custom_command(
    TARGET bof3_exe POST_BUILD
    COMMAND
        ${BOF3_ELF2X} -q
        $<SHELL_PATH:$<TARGET_FILE:bof3_exe>>
        $<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/bof3.exe>
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/bof3.exe
    VERBATIM
)

include(${BOF3_CMAKE_DIR}/artifacts.cmake)

get_property(BOF3_REGISTERED_ARTIFACT_TARGETS GLOBAL PROPERTY BOF3_ARTIFACT_TARGETS)
if(BOF3_REGISTERED_ARTIFACT_TARGETS)
    bof3_artifact_write_manifest(BOF3_ARTIFACT_MANIFEST)
    add_custom_target(artifact_manifest DEPENDS "${BOF3_ARTIFACT_MANIFEST}")
    add_custom_target(artifacts)
    add_dependencies(artifacts artifact_manifest)
    add_dependencies(artifacts ${BOF3_REGISTERED_ARTIFACT_TARGETS})
endif()
