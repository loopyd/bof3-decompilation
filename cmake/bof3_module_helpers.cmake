include_guard(GLOBAL)

function(bof3_collect_sources_with_prefix out_var)
    set(result)
    foreach(source IN LISTS BOF3_BOOT_SOURCES BOF3_SOURCES)
        foreach(prefix ${ARGN})
            string(FIND "${source}" "${prefix}" prefix_index)
            if(prefix_index EQUAL 0)
                list(APPEND result "${source}")
                break()
            endif()
        endforeach()
    endforeach()
    list(REMOVE_DUPLICATES result)
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

function(bof3_define_module_artifact target)
    cmake_parse_arguments(
        ARG
        "PLACEHOLDER"
        "DISC_FOLDER;PROGRAM_NAME;PROGRAM_PATH;SOURCE_HINT;KIND"
        "SOURCE_PREFIXES;DECLARED_SOURCES"
        ${ARGN}
    )

    if(NOT ARG_PROGRAM_NAME OR NOT ARG_PROGRAM_PATH OR NOT ARG_SOURCE_HINT OR NOT ARG_DISC_FOLDER)
        message(FATAL_ERROR
            "bof3_define_module_artifact requires DISC_FOLDER, PROGRAM_NAME, PROGRAM_PATH, and SOURCE_HINT."
        )
    endif()

    set(kind "${ARG_KIND}")
    if(kind STREQUAL "")
        set(kind "module")
    endif()

    set(declared_sources ${ARG_DECLARED_SOURCES})
    if(NOT declared_sources)
        if(NOT ARG_SOURCE_PREFIXES)
            message(FATAL_ERROR
                "bof3_define_module_artifact requires SOURCE_PREFIXES or DECLARED_SOURCES."
            )
        endif()
        bof3_collect_sources_with_prefix(declared_sources ${ARG_SOURCE_PREFIXES})
    endif()

    if(ARG_PLACEHOLDER)
        bof3_artifact_register_placeholder(
            "${target}"
            FOLDER "${ARG_DISC_FOLDER}"
            PROGRAM_NAME "${ARG_PROGRAM_NAME}"
            PROGRAM_PATH "${ARG_PROGRAM_PATH}"
            SOURCE_HINT "${ARG_SOURCE_HINT}"
            KIND "${kind}"
            DECLARED_SOURCES ${declared_sources}
        )
    else()
        bof3_artifact_register_archive(
            "${target}"
            FOLDER "${ARG_DISC_FOLDER}"
            PROGRAM_NAME "${ARG_PROGRAM_NAME}"
            PROGRAM_PATH "${ARG_PROGRAM_PATH}"
            SOURCE_HINT "${ARG_SOURCE_HINT}"
            KIND "${kind}"
            DECLARED_SOURCES ${declared_sources}
        )
    endif()
endfunction()
