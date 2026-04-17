include_guard(GLOBAL)

function(bof3_require_psx_profile expected_profile)
    if(NOT BOF3_PSX_PROFILE STREQUAL expected_profile)
        message(FATAL_ERROR "bof3 requires the ${expected_profile} profile.")
    endif()
endfunction()

function(bof3_require_psyq_layout)
    if(NOT DEFINED BOF3_PSYQ_INCLUDE_DIR OR NOT DEFINED BOF3_PSYQ_LIB_DIR)
        message(FATAL_ERROR
            "BOF3_PSYQ_INCLUDE_DIR and BOF3_PSYQ_LIB_DIR must be set by the selected profile preset."
        )
    endif()

    if(NOT EXISTS "${BOF3_PSYQ_INCLUDE_DIR}/libgpu.h")
        message(FATAL_ERROR "PsyQ headers not found at ${BOF3_PSYQ_INCLUDE_DIR}.")
    endif()

    if(NOT EXISTS "${BOF3_PSYQ_LIB_DIR}")
        message(FATAL_ERROR "Original PsyQ library directory not found at ${BOF3_PSYQ_LIB_DIR}.")
    endif()
endfunction()

function(bof3_find_sdk_tool out_var tool_name)
    set(search_roots)
    if(DEFINED BOF3_PSN00B_SDK_ROOT AND IS_DIRECTORY "${BOF3_PSN00B_SDK_ROOT}")
        list(APPEND search_roots "${BOF3_PSN00B_SDK_ROOT}")
        file(GLOB sdk_children LIST_DIRECTORIES true "${BOF3_PSN00B_SDK_ROOT}/*")
        foreach(child IN LISTS sdk_children)
            if(IS_DIRECTORY "${child}")
                list(APPEND search_roots "${child}")
            endif()
        endforeach()
    endif()

    foreach(root IN LISTS search_roots)
        set(candidate "${root}/bin/${tool_name}")
        if(EXISTS "${candidate}")
            set(${out_var} "${candidate}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    find_program(found_tool NAMES "${tool_name}")
    if(found_tool)
        set(${out_var} "${found_tool}" PARENT_SCOPE)
        return()
    endif()

    message(FATAL_ERROR
        "Required PSX SDK tool `${tool_name}` was not found under ${BOF3_PSN00B_SDK_ROOT} or in PATH."
    )
endfunction()
