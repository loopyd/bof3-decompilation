include_guard(GLOBAL)

function(bof3_apply_common_target_settings target)
    cmake_parse_arguments(ARG "" "INCLUDE_VISIBILITY" "COMPILE_DEFINITIONS" ${ARGN})

    if(NOT ARG_INCLUDE_VISIBILITY)
        set(ARG_INCLUDE_VISIBILITY PRIVATE)
    endif()
    if(NOT DEFINED BOF3_INCLUDE_DIR)
        message(FATAL_ERROR "BOF3_INCLUDE_DIR must be set before bof3_apply_common_target_settings().")
    endif()

    target_include_directories("${target}" ${ARG_INCLUDE_VISIBILITY}
        "${BOF3_INCLUDE_DIR}"
        "${BOF3_PSYQ_INCLUDE_DIR}"
    )
    target_compile_definitions("${target}" PRIVATE
        BOF3_TARGET_PSX=1
        ${ARG_COMPILE_DEFINITIONS}
    )
endfunction()
