include_guard(GLOBAL)

function(bof3_require_psyq_layout)
    if(NOT DEFINED BOF3_PSYQ_INCLUDE_DIR OR NOT DEFINED BOF3_PSYQ_LIB_DIR)
        message(FATAL_ERROR
            "The selected PSX toolchain must define BOF3_PSYQ_INCLUDE_DIR and BOF3_PSYQ_LIB_DIR."
        )
    endif()
    if(NOT EXISTS "${BOF3_PSYQ_INCLUDE_DIR}/libgpu.h")
        message(FATAL_ERROR "PsyQ headers not found at ${BOF3_PSYQ_INCLUDE_DIR}.")
    endif()
    if(NOT IS_DIRECTORY "${BOF3_PSYQ_LIB_DIR}")
        message(FATAL_ERROR "PsyQ libraries not found at ${BOF3_PSYQ_LIB_DIR}.")
    endif()
endfunction()

function(bof3_find_sdk_tool out_var tool_name)
    set(search_roots "${BOF3_PSN00B_SDK_ROOT}")
    if(IS_DIRECTORY "${BOF3_PSN00B_SDK_ROOT}")
        file(GLOB sdk_children LIST_DIRECTORIES true "${BOF3_PSN00B_SDK_ROOT}/*")
        list(APPEND search_roots ${sdk_children})
    endif()

    foreach(root IN LISTS search_roots)
        if(EXISTS "${root}/bin/${tool_name}")
            set(${out_var} "${root}/bin/${tool_name}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    message(FATAL_ERROR
        "Required PSX SDK tool `${tool_name}` was not found under ${BOF3_PSN00B_SDK_ROOT}. Run 'just setup'."
    )
endfunction()

function(bof3_apply_target_settings target)
    cmake_parse_arguments(ARG "" "INCLUDE_VISIBILITY" "COMPILE_DEFINITIONS" ${ARGN})
    if(NOT ARG_INCLUDE_VISIBILITY)
        set(ARG_INCLUDE_VISIBILITY PRIVATE)
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
