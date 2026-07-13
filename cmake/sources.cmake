include_guard(GLOBAL)

# Source ownership is generated from config/targets/*.toml.  Keeping this
# adapter tiny prevents CMake and the Python registry from drifting apart.
set(HARNESS_GENERATED_SOURCES "${HARNESS_ROOT_DIR}/out/build/sources.cmake")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "PYTHONPATH=${HARNESS_ROOT_DIR}/tools/python"
        "${Python3_EXECUTABLE}" -c
        "from pathlib import Path; from harness.registry import generate_build_manifest; generate_build_manifest(Path('.'))"
    WORKING_DIRECTORY "${HARNESS_ROOT_DIR}"
    RESULT_VARIABLE HARNESS_SOURCE_REGISTRY_RESULT
)
if(NOT HARNESS_SOURCE_REGISTRY_RESULT EQUAL 0 OR NOT EXISTS "${HARNESS_GENERATED_SOURCES}")
    message(FATAL_ERROR "failed to generate ${HARNESS_GENERATED_SOURCES} from target manifests")
endif()
include("${HARNESS_GENERATED_SOURCES}")

set(HARNESS_CORE_SOURCES ${HARNESS_TARGET_EXE_SLUS_004_22_SOURCES})
list(FILTER HARNESS_CORE_SOURCES EXCLUDE REGEX "/startup\\.s$")
list(FILTER HARNESS_CORE_SOURCES EXCLUDE REGEX "/symbols(/[^/]+)?\\.c$")
# This historical probe duplicates LOGO.EXE behavior and calls LOGO-local
# functions. Preserve it as investigation evidence, but never link it into the
# independently loaded SLUS target.
list(FILTER HARNESS_CORE_SOURCES EXCLUDE REGEX "/slot_table_logo_str\\.c$")
