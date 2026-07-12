include_guard(GLOBAL)

function(harness_add_artifact target)
    cmake_parse_arguments(
        ARG
        "PLACEHOLDER;RAW_BINARY"
        "DISC_FOLDER;PROGRAM_NAME;PROGRAM_PATH;SOURCE_HINT;KIND;LOAD_ADDRESS;RAW_SIZE"
        "DECLARED_SOURCES"
        ${ARGN}
    )

    if(NOT ARG_PROGRAM_NAME OR NOT ARG_PROGRAM_PATH OR NOT ARG_SOURCE_HINT OR NOT ARG_DISC_FOLDER)
        message(FATAL_ERROR
            "harness_add_artifact requires DISC_FOLDER, PROGRAM_NAME, PROGRAM_PATH, and SOURCE_HINT."
        )
    endif()

    set(kind "${ARG_KIND}")
    if(kind STREQUAL "")
        set(kind "module")
    endif()

    if(NOT ARG_DECLARED_SOURCES AND NOT ARG_PLACEHOLDER)
        message(FATAL_ERROR
            "harness_add_artifact requires DECLARED_SOURCES."
        )
    endif()

    if(ARG_PLACEHOLDER)
        harness_artifact_register_placeholder(
            "${target}"
            FOLDER "${ARG_DISC_FOLDER}"
            PROGRAM_NAME "${ARG_PROGRAM_NAME}"
            PROGRAM_PATH "${ARG_PROGRAM_PATH}"
            SOURCE_HINT "${ARG_SOURCE_HINT}"
            KIND "${kind}"
            DECLARED_SOURCES ${ARG_DECLARED_SOURCES}
        )
    elseif(ARG_RAW_BINARY)
        harness_artifact_register_raw_module(
            "${target}"
            FOLDER "${ARG_DISC_FOLDER}"
            PROGRAM_NAME "${ARG_PROGRAM_NAME}"
            PROGRAM_PATH "${ARG_PROGRAM_PATH}"
            SOURCE_HINT "${ARG_SOURCE_HINT}"
            KIND "${kind}"
            LOAD_ADDRESS "${ARG_LOAD_ADDRESS}"
            RAW_SIZE "${ARG_RAW_SIZE}"
            DECLARED_SOURCES ${ARG_DECLARED_SOURCES}
        )
    else()
        harness_artifact_register_archive(
            "${target}"
            FOLDER "${ARG_DISC_FOLDER}"
            PROGRAM_NAME "${ARG_PROGRAM_NAME}"
            PROGRAM_PATH "${ARG_PROGRAM_PATH}"
            SOURCE_HINT "${ARG_SOURCE_HINT}"
            KIND "${kind}"
            DECLARED_SOURCES ${ARG_DECLARED_SOURCES}
        )
    endif()
endfunction()
