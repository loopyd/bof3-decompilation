include_guard(GLOBAL)

unset(HARNESS_ARTIFACT_ROOT_DIR CACHE)
set(HARNESS_ARTIFACT_ROOT_DIR "${CMAKE_BINARY_DIR}/artifacts")

set(HARNESS_ARTIFACT_RAW_ROOT_DIR "${HARNESS_ARTIFACT_ROOT_DIR}/raw")
set(HARNESS_ARTIFACT_METADATA_DIR "${HARNESS_ARTIFACT_ROOT_DIR}/metadata")

function(harness_artifact_normalize_folder out_var folder)
    string(REGEX REPLACE "^/+" "" normalized "${folder}")
    string(REGEX REPLACE "/+$" "" normalized "${normalized}")
    set(${out_var} "${normalized}" PARENT_SCOPE)
endfunction()

function(harness_artifact_resolve_paths out_relative_path out_program_path out_raw_output_path folder program_name)
    harness_artifact_normalize_folder(normalized_folder "${folder}")
    if(normalized_folder STREQUAL "")
        set(relative_path "${program_name}")
    else()
        set(relative_path "${normalized_folder}/${program_name}")
    endif()
    set(program_path "/${relative_path}")
    set(raw_output_path "${HARNESS_ARTIFACT_RAW_ROOT_DIR}/${relative_path}")

    set(${out_relative_path} "${relative_path}" PARENT_SCOPE)
    set(${out_program_path} "${program_path}" PARENT_SCOPE)
    set(${out_raw_output_path} "${raw_output_path}" PARENT_SCOPE)
endfunction()

function(harness_artifact_escape_json_string out_var value)
    string(REPLACE "\\" "\\\\" escaped "${value}")
    string(REPLACE "\"" "\\\"" escaped "${escaped}")
    string(REPLACE "\n" "\\n" escaped "${escaped}")
    set(${out_var} "${escaped}" PARENT_SCOPE)
endfunction()

function(harness_artifact_get_target_property_or_empty out_var target property_name)
    get_target_property(value "${target}" "${property_name}")
    if(value STREQUAL "${property_name}-NOTFOUND")
        set(value "")
    endif()
    set(${out_var} "${value}" PARENT_SCOPE)
endfunction()

function(harness_artifact_register target)
    cmake_parse_arguments(
        ARG
        "PLACEHOLDER"
        "FOLDER;PROGRAM_NAME;PROGRAM_PATH;SOURCE_HINT;KIND;BUILD_STAGE;OUTPUT_PATH"
        "DECLARED_SOURCES"
        ${ARGN}
    )

    if(NOT TARGET "${target}")
        message(FATAL_ERROR "Artifact target `${target}` must exist before registration.")
    endif()
    if(NOT DEFINED ARG_PROGRAM_NAME OR ARG_PROGRAM_NAME STREQUAL "")
        message(FATAL_ERROR "harness_artifact_register requires PROGRAM_NAME.")
    endif()

    harness_artifact_resolve_paths(relative_path computed_program_path raw_output_path
        "${ARG_FOLDER}" "${ARG_PROGRAM_NAME}")
    set(program_path "${ARG_PROGRAM_PATH}")
    if(program_path STREQUAL "")
        set(program_path "${computed_program_path}")
    endif()

    set_target_properties("${target}" PROPERTIES
        HARNESS_ARTIFACT_KIND "${ARG_KIND}"
        HARNESS_ARTIFACT_PROGRAM_PATH "${program_path}"
        HARNESS_ARTIFACT_SOURCE_HINT "${ARG_SOURCE_HINT}"
        HARNESS_ARTIFACT_BUILD_STAGE "${ARG_BUILD_STAGE}"
        HARNESS_ARTIFACT_OUTPUT_PATH "${ARG_OUTPUT_PATH}"
        HARNESS_ARTIFACT_PLACEHOLDER "${ARG_PLACEHOLDER}"
    )
    set_property(GLOBAL APPEND PROPERTY HARNESS_ARTIFACT_TARGETS "${target}")
endfunction()

function(harness_artifact_register_placeholder target)
    cmake_parse_arguments(
        ARG
        ""
        "FOLDER;PROGRAM_NAME;PROGRAM_PATH;SOURCE_HINT;KIND"
        "DECLARED_SOURCES"
        ${ARGN}
    )

    add_custom_target("${target}")
    harness_artifact_register(
        "${target}"
        PLACEHOLDER
        FOLDER "${ARG_FOLDER}"
        PROGRAM_NAME "${ARG_PROGRAM_NAME}"
        PROGRAM_PATH "${ARG_PROGRAM_PATH}"
        SOURCE_HINT "${ARG_SOURCE_HINT}"
        KIND "${ARG_KIND}"
        BUILD_STAGE "placeholder"
        DECLARED_SOURCES ${ARG_DECLARED_SOURCES}
    )
endfunction()

function(harness_artifact_register_built target)
    cmake_parse_arguments(
        ARG
        ""
        "FOLDER;PROGRAM_NAME;PROGRAM_PATH;SOURCE_HINT;KIND;BUILT_OUTPUT"
        "DECLARED_SOURCES"
        ${ARGN}
    )

    if(NOT ARG_BUILT_OUTPUT)
        message(FATAL_ERROR "harness_artifact_register_built requires BUILT_OUTPUT.")
    endif()

    harness_artifact_resolve_paths(relative_path computed_program_path raw_output_path
        "${ARG_FOLDER}" "${ARG_PROGRAM_NAME}")
    set(program_path "${ARG_PROGRAM_PATH}")
    if(program_path STREQUAL "")
        set(program_path "${computed_program_path}")
    endif()

    harness_artifact_register(
        "${target}"
        FOLDER "${ARG_FOLDER}"
        PROGRAM_NAME "${ARG_PROGRAM_NAME}"
        PROGRAM_PATH "${program_path}"
        SOURCE_HINT "${ARG_SOURCE_HINT}"
        KIND "${ARG_KIND}"
        BUILD_STAGE "raw"
        OUTPUT_PATH "${raw_output_path}"
        DECLARED_SOURCES ${ARG_DECLARED_SOURCES}
    )

    get_filename_component(raw_output_dir "${raw_output_path}" DIRECTORY)
    add_custom_command(
        TARGET "${target}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<SHELL_PATH:${raw_output_dir}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<SHELL_PATH:${ARG_BUILT_OUTPUT}>"
            "$<SHELL_PATH:${raw_output_path}>"
        BYPRODUCTS "${raw_output_path}"
        VERBATIM
    )
endfunction()

function(harness_artifact_register_archive target)
    cmake_parse_arguments(
        ARG
        ""
        "FOLDER;PROGRAM_NAME;PROGRAM_PATH;SOURCE_HINT;KIND"
        "DECLARED_SOURCES"
        ${ARGN}
    )

    if(NOT ARG_DECLARED_SOURCES)
        message(FATAL_ERROR "harness_artifact_register_archive requires DECLARED_SOURCES.")
    endif()

    harness_artifact_normalize_folder(normalized_folder "${ARG_FOLDER}")
    if(normalized_folder STREQUAL "")
        set(output_dir "${HARNESS_ARTIFACT_ROOT_DIR}/compiled")
    else()
        set(output_dir "${HARNESS_ARTIFACT_ROOT_DIR}/compiled/${normalized_folder}")
    endif()
    file(MAKE_DIRECTORY "${output_dir}")

    add_library("${target}" STATIC EXCLUDE_FROM_ALL ${ARG_DECLARED_SOURCES})
    harness_apply_target_settings("${target}")
    set_target_properties("${target}" PROPERTIES
        PREFIX ""
        OUTPUT_NAME "${ARG_PROGRAM_NAME}"
        ARCHIVE_OUTPUT_DIRECTORY "${output_dir}"
    )

    harness_artifact_register(
        "${target}"
        FOLDER "${ARG_FOLDER}"
        PROGRAM_NAME "${ARG_PROGRAM_NAME}"
        PROGRAM_PATH "${ARG_PROGRAM_PATH}"
        SOURCE_HINT "${ARG_SOURCE_HINT}"
        KIND "${ARG_KIND}"
        BUILD_STAGE "archive"
        OUTPUT_PATH "${output_dir}/${ARG_PROGRAM_NAME}.a"
        DECLARED_SOURCES ${ARG_DECLARED_SOURCES}
    )
endfunction()

function(harness_artifact_register_raw_module target)
    cmake_parse_arguments(
        ARG
        ""
        "FOLDER;PROGRAM_NAME;PROGRAM_PATH;SOURCE_HINT;KIND;LOAD_ADDRESS;RAW_SIZE"
        "DECLARED_SOURCES"
        ${ARGN}
    )

    if(NOT ARG_DECLARED_SOURCES)
        message(FATAL_ERROR "harness_artifact_register_raw_module requires DECLARED_SOURCES.")
    endif()
    if(NOT ARG_LOAD_ADDRESS)
        message(FATAL_ERROR "harness_artifact_register_raw_module requires LOAD_ADDRESS.")
    endif()
    if(NOT CMAKE_OBJCOPY)
        message(FATAL_ERROR "CMAKE_OBJCOPY must be set for raw module artifacts.")
    endif()
    if(NOT Python3_EXECUTABLE)
        message(FATAL_ERROR "Python3_EXECUTABLE must be set for raw module artifacts.")
    endif()

    set(object_target "${target}_objects")
    add_library("${object_target}" OBJECT EXCLUDE_FROM_ALL ${ARG_DECLARED_SOURCES})
    harness_apply_target_settings("${object_target}")

    harness_artifact_resolve_paths(relative_path computed_program_path raw_output_path
        "${ARG_FOLDER}" "${ARG_PROGRAM_NAME}")
    set(program_path "${ARG_PROGRAM_PATH}")
    if(program_path STREQUAL "")
        set(program_path "${computed_program_path}")
    endif()
    set(metadata_path "${raw_output_path}.json")
    get_filename_component(raw_output_dir "${raw_output_path}" DIRECTORY)

    set(size_args)
    if(ARG_RAW_SIZE)
        list(APPEND size_args "--size" "${ARG_RAW_SIZE}")
    endif()

    add_custom_command(
        OUTPUT "${raw_output_path}" "${metadata_path}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<SHELL_PATH:${raw_output_dir}>"
        COMMAND ${CMAKE_COMMAND} -E env
            "PYTHONPATH=$<SHELL_PATH:${HARNESS_ROOT_DIR}/tools/python>"
            "$<SHELL_PATH:${Python3_EXECUTABLE}>"
            -m harness.build.raw_module
            --output "$<SHELL_PATH:${raw_output_path}>"
            --metadata "$<SHELL_PATH:${metadata_path}>"
            --base-address "${ARG_LOAD_ADDRESS}"
            ${size_args}
            --truncate-overlaps
            --objcopy "$<SHELL_PATH:${CMAKE_OBJCOPY}>"
            --objects $<TARGET_OBJECTS:${object_target}>
        DEPENDS "${object_target}"
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
    add_custom_target("${target}" DEPENDS "${raw_output_path}" "${metadata_path}")
    add_dependencies("${target}" "${object_target}")

    set(kind "${ARG_KIND}")
    if(kind STREQUAL "")
        set(kind "module")
    endif()
    harness_artifact_register(
        "${target}"
        FOLDER "${ARG_FOLDER}"
        PROGRAM_NAME "${ARG_PROGRAM_NAME}"
        PROGRAM_PATH "${program_path}"
        SOURCE_HINT "${ARG_SOURCE_HINT}"
        KIND "${kind}"
        BUILD_STAGE "raw"
        OUTPUT_PATH "${raw_output_path}"
        DECLARED_SOURCES ${ARG_DECLARED_SOURCES}
    )
endfunction()

function(harness_artifact_write_manifest out_var)
    get_property(registered_targets GLOBAL PROPERTY HARNESS_ARTIFACT_TARGETS)

    set(manifest_path "${HARNESS_ARTIFACT_METADATA_DIR}/artifacts.json")
    get_filename_component(manifest_dir "${manifest_path}" DIRECTORY)
    file(MAKE_DIRECTORY "${manifest_dir}")

    file(WRITE "${manifest_path}" "{\n  \"artifacts\": [")

    set(needs_separator OFF)
    foreach(target IN LISTS registered_targets)
        harness_artifact_get_target_property_or_empty(kind "${target}" HARNESS_ARTIFACT_KIND)
        harness_artifact_get_target_property_or_empty(program_path "${target}" HARNESS_ARTIFACT_PROGRAM_PATH)
        harness_artifact_get_target_property_or_empty(source_hint "${target}" HARNESS_ARTIFACT_SOURCE_HINT)
        harness_artifact_get_target_property_or_empty(build_stage "${target}" HARNESS_ARTIFACT_BUILD_STAGE)
        harness_artifact_get_target_property_or_empty(output_path "${target}" HARNESS_ARTIFACT_OUTPUT_PATH)
        get_target_property(placeholder "${target}" HARNESS_ARTIFACT_PLACEHOLDER)

        if(placeholder)
            set(is_placeholder "true")
        else()
            set(is_placeholder "false")
        endif()

        harness_artifact_escape_json_string(escaped_target "${target}")
        harness_artifact_escape_json_string(escaped_kind "${kind}")
        harness_artifact_escape_json_string(escaped_program_path "${program_path}")
        harness_artifact_escape_json_string(escaped_source_hint "${source_hint}")
        harness_artifact_escape_json_string(escaped_build_stage "${build_stage}")
        harness_artifact_escape_json_string(escaped_output_path "${output_path}")

        if(needs_separator)
            file(APPEND "${manifest_path}" ",")
        endif()
        file(APPEND "${manifest_path}"
            "\n    {\n"
            "      \"target\": \"${escaped_target}\",\n"
            "      \"kind\": \"${escaped_kind}\",\n"
            "      \"program_path\": \"${escaped_program_path}\",\n"
            "      \"build_stage\": \"${escaped_build_stage}\",\n"
            "      \"output\": \"${escaped_output_path}\",\n"
            "      \"source_hint\": \"${escaped_source_hint}\",\n"
            "      \"placeholder\": ${is_placeholder}\n"
            "    }"
        )
        set(needs_separator ON)
    endforeach()

    if(needs_separator)
        file(APPEND "${manifest_path}" "\n")
    endif()
    file(APPEND "${manifest_path}" "  ]\n}\n")

    set(${out_var} "${manifest_path}" PARENT_SCOPE)
endfunction()
