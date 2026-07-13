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

option(HARNESS_BUILD_RUST_TOOLS "Expose canonical Rust extraction tool targets" ON)
if(HARNESS_BUILD_RUST_TOOLS)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/rust-tools.cmake")
endif()

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
