include_guard(GLOBAL)

# Artifact target registration is generated from config/targets/*.toml.  Keeping
# this adapter tiny prevents CMake and the Python registry from drifting apart.
set(HARNESS_GENERATED_TARGETS "${HARNESS_ROOT_DIR}/out/build/targets.cmake")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "PYTHONPATH=${HARNESS_ROOT_DIR}/tools/python"
        "${Python3_EXECUTABLE}" -c
        "from pathlib import Path; from harness.registry import generate_target_manifest; generate_target_manifest(Path('.'))"
    WORKING_DIRECTORY "${HARNESS_ROOT_DIR}"
    RESULT_VARIABLE HARNESS_TARGET_REGISTRY_RESULT
)
if(NOT HARNESS_TARGET_REGISTRY_RESULT EQUAL 0 OR NOT EXISTS "${HARNESS_GENERATED_TARGETS}")
    message(FATAL_ERROR "failed to generate ${HARNESS_GENERATED_TARGETS} from target manifests")
endif()
include("${HARNESS_GENERATED_TARGETS}")
