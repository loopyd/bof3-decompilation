cmake_minimum_required(VERSION 3.20)

project(bof3_lifts NONE)

set(BOF3_PROFILE "compat/capcom97" CACHE STRING "Lift compiler profile")
if(NOT BOF3_PROFILE STREQUAL "compat/capcom97")
  message(FATAL_ERROR "unsupported BOF3_PROFILE=${BOF3_PROFILE}; only compat/capcom97 is implemented")
endif()

set(BOF3_OUTPUT_ROOT "${CMAKE_SOURCE_DIR}/build" CACHE PATH "Generated object root")
set(BOF3_CC "${CMAKE_SOURCE_DIR}/bin/cc")
set(BOF3_AS "${CMAKE_SOURCE_DIR}/bin/as")
set(BOF3_CPPFLAGS
    -DHARNESS_TARGET_PSX=1
    -I${CMAKE_SOURCE_DIR}/src
    -I${CMAKE_SOURCE_DIR}/include
    -I${CMAKE_SOURCE_DIR}/toolchains/psyq/4.7/include)
set(BOF3_CFLAGS -O2 -G0 -funsigned-char -msoft-float -gcoff)
set(BOF3_CC_ASFLAGS -Wa,--aspsx-version=2.56 -Wa,-G0,-EL,-mips1)
set(BOF3_ASFLAGS -G0 -EL -mips1)

# Per-object compiler-flag overrides. BOF3_CFLAGS_BASE is the canonical set with
# the -O level removed; an override re-adds the verified optimization/scheduling
# flags for one source (see config/compiler/object-flags.cmake). Sources without
# an override compile with the untouched canonical BOF3_CFLAGS.
set(BOF3_CFLAGS_BASE ${BOF3_CFLAGS})
list(FILTER BOF3_CFLAGS_BASE EXCLUDE REGEX "^-O")
set(BOF3_OBJECT_FLAGS_FILE "${CMAKE_SOURCE_DIR}/config/compiler/object-flags.cmake")
set(BOF3_VARIANTS_FILE "${CMAKE_SOURCE_DIR}/config/compiler/variants.json")
include("${BOF3_OBJECT_FLAGS_FILE}" OPTIONAL)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${BOF3_OBJECT_FLAGS_FILE}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${BOF3_VARIANTS_FILE}")

file(GLOB_RECURSE BOF3_C_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE BOF3_ASM_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/src/*.s" "${CMAKE_SOURCE_DIR}/src/*.S")
file(GLOB_RECURSE BOF3_SHARED_HEADERS CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/include/*.h" "${CMAKE_SOURCE_DIR}/include/*.inc")
file(GLOB_RECURSE BOF3_SHARED_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/src/shared/*.h" "${CMAKE_SOURCE_DIR}/src/shared/*.inc")

set(BOF3_OBJECTS)
set(BOF3_SOURCE_DIRECTORIES)

# Canonical target roots: every manifest's source_dir (e.g. src/emi/...).
# Nested subsystem folders below a root share that root's build target so
# `bin/build TARGET` compiles every descendant lift.
set(BOF3_MANIFEST_SOURCE_DIRS)
file(GLOB_RECURSE BOF3_MANIFEST_TOMLS CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/config/targets/*/target.toml")
foreach(manifest_file IN LISTS BOF3_MANIFEST_TOMLS)
  file(READ "${manifest_file}" _manifest_text)
  if(_manifest_text MATCHES "source_dir[ \t]*=[ \t]*\"([^\"]+)\"")
    list(APPEND BOF3_MANIFEST_SOURCE_DIRS "${CMAKE_MATCH_1}")
  endif()
endforeach()
list(REMOVE_DUPLICATES BOF3_MANIFEST_SOURCE_DIRS)

# Explicit target source claims (manifest sources/support_sources/headers):
# an authored lift may live in a semantic src/bof3/<class>/ folder far outside
# the manifest source_dir, so grouping uses the claim, never path ancestry.
# Unmigrated targets emit no claims and keep the legacy inventory grouping.
if(EXISTS "${CMAKE_SOURCE_DIR}/.venv/bin/python")
  set(BOF3_CLAIMS_PYTHON "${CMAKE_SOURCE_DIR}/.venv/bin/python")
else()
  find_program(BOF3_CLAIMS_PYTHON python3)
endif()
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=${CMAKE_SOURCE_DIR}/tools/python"
          "${BOF3_CLAIMS_PYTHON}" -m harness.commands.manifest_claims --root "${CMAKE_SOURCE_DIR}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE _manifest_claims
  RESULT_VARIABLE _claims_result
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _claims_result EQUAL 0)
  message(FATAL_ERROR "manifest claims generation failed (${_claims_result})")
endif()
set(BOF3_CLAIMED_PATHS)
set(BOF3_CLAIMED_OWNERS)
set(BOF3_CLAIMED_HEADER_PATHS)
set(BOF3_CLAIMED_HEADER_OWNERS)
set(BOF3_MIGRATED_SOURCE_DIRS)
# execute_process stores output as one list element with embedded newlines;
# split it into lines explicitly so every claim parses independently.
string(REPLACE "\n" ";" _manifest_lines "${_manifest_claims}")
foreach(_line IN LISTS _manifest_lines)
  if(_line STREQUAL "")
    continue()
  endif()
  string(REPLACE "|" ";" _fields "${_line}")
  list(LENGTH _fields _nfields)
  if(_nfields LESS 3)
    continue()
  endif()
  list(GET _fields 0 _kind)
  list(GET _fields 1 _owner)
  list(GET _fields 2 _path)
  if(_kind STREQUAL "source")
    list(APPEND BOF3_CLAIMED_PATHS "${_path}")
    list(APPEND BOF3_CLAIMED_OWNERS "${_owner}")
  elseif(_kind STREQUAL "header")
    list(APPEND BOF3_CLAIMED_HEADER_PATHS "${_path}")
    list(APPEND BOF3_CLAIMED_HEADER_OWNERS "${_owner}")
  elseif(_kind STREQUAL "migrated")
    # Explicit-claim target: unclaimed files under its legacy source_dir are
    # not owned by it and must never join its build target.
    list(APPEND BOF3_MIGRATED_SOURCE_DIRS "${_owner}")
  endif()
endforeach()

foreach(source IN LISTS BOF3_C_SOURCES BOF3_ASM_SOURCES)
  file(RELATIVE_PATH relative "${CMAKE_SOURCE_DIR}/src" "${source}")
  string(REGEX REPLACE "\.[^.]+$" ".o" relative_object "${relative}")
  set(object "${BOF3_OUTPUT_ROOT}/src/${relative_object}")
  get_filename_component(object_directory "${object}" DIRECTORY)

  get_filename_component(extension "${source}" EXT)
  get_filename_component(source_parent "${source}" DIRECTORY)
  file(GLOB source_headers CONFIGURE_DEPENDS "${source_parent}/*.h")
  if(extension STREQUAL ".c")
    string(MAKE_C_IDENTIFIER "${relative}" object_flags_key)
    if(DEFINED BOF3_OBJFLAGS_${object_flags_key})
      set(object_cflags ${BOF3_CFLAGS_BASE} ${BOF3_OBJFLAGS_${object_flags_key}})
    else()
      set(object_cflags ${BOF3_CFLAGS})
    endif()
    # Per-object compiler variant override — wraps bin/cc with cmake -E env.
    if(DEFINED BOF3_OBJCOMPILER_${object_flags_key})
      set(_variant_id ${BOF3_OBJCOMPILER_${object_flags_key}})
      execute_process(
        COMMAND "${CMAKE_SOURCE_DIR}/bin/compiler-variants" "path" "${_variant_id}"
        OUTPUT_VARIABLE _variant_gcc_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _variant_result
        ERROR_VARIABLE _variant_error
      )
      if(NOT _variant_result EQUAL 0)
        message(FATAL_ERROR "${_variant_error}")
      endif()
      set(_cc_command
        "${CMAKE_COMMAND}" -E env "PSX_GCC=${_variant_gcc_path}"
        "${BOF3_CC}")
    else()
      set(_cc_command "${BOF3_CC}")
    endif()
    add_custom_command(
      OUTPUT "${object}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${object_directory}"
      COMMAND ${_cc_command} ${BOF3_CPPFLAGS} ${object_cflags} ${BOF3_CC_ASFLAGS}
              -c "${source}" -o "${object}"
      DEPENDS "${source}" ${source_headers} ${BOF3_SHARED_HEADERS}
              ${BOF3_SHARED_SOURCES}
      COMMENT "Compiling ${relative}"
      VERBATIM)
  else()
    add_custom_command(
      OUTPUT "${object}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${object_directory}"
      COMMAND "${BOF3_AS}" ${BOF3_ASFLAGS} -o "${object}" "${source}"
      DEPENDS "${source}"
      COMMENT "Assembling ${relative}"
      VERBATIM)
  endif()

  # The public bin/build command resolves friendly function IDs to this stable
  # internal target; callers never need to know the encoded target name.
  string(SHA1 target_hash "src/${relative}")
  string(SUBSTRING "${target_hash}" 0 16 target_hash)
  add_custom_target("lift_${target_hash}" DEPENDS "${object}")
  list(APPEND BOF3_OBJECTS "${object}")

  # Group every source under the deepest owning manifest source_dir so a
  # relocated (nested) lift still builds with `bin/build TARGET`.  An explicit
  # manifest claim wins over path containment and may point far outside every
  # manifest root (semantic src/bof3/<class>/ folders); directories outside
  # any manifest root without a claim keep their own grouping.  For a target
  # with explicit claims (migrated), its build target owns exactly its claimed
  # translation units: unclaimed files under its legacy source_dir get no
  # target group (the global `lifts` target still compiles them).
  get_filename_component(source_directory "src/${relative}" DIRECTORY)
  set(group_directory "${source_directory}")
  set(_claim_owner "")
  list(FIND BOF3_CLAIMED_PATHS "src/${relative}" _claim_index)
  if(_claim_index GREATER -1)
    list(GET BOF3_CLAIMED_OWNERS ${_claim_index} _claim_owner)
    set(group_directory "${_claim_owner}")
  else()
    set(_best_prefix_len -1)
    set(_best_manifest_dir "")
    foreach(manifest_dir IN LISTS BOF3_MANIFEST_SOURCE_DIRS)
      string(LENGTH "${manifest_dir}" _manifest_len)
      string(FIND "${source_directory}/" "${manifest_dir}/" _prefix_at)
      if(_prefix_at EQUAL 0 AND _manifest_len GREATER _best_prefix_len)
        set(_best_prefix_len "${_manifest_len}")
        set(_best_manifest_dir "${manifest_dir}")
      endif()
    endforeach()
    if(_best_manifest_dir)
      list(FIND BOF3_MIGRATED_SOURCE_DIRS "${_best_manifest_dir}" _migrated_index)
      if(_migrated_index EQUAL -1)
        set(group_directory "${_best_manifest_dir}")
      else()
        # Explicit-claim target: unclaimed files under its legacy source_dir
        # are not owned by it. Keep them compiling via the global `lifts`
        # target but give them no target group (no bin/build TARGET claim).
        set(group_directory "")
      endif()
    endif()
  endif()
  if(_claim_owner)
    # Claimed header deps: every explicit private header of the owning target.
    set(_i 0)
    foreach(_h IN LISTS BOF3_CLAIMED_HEADER_PATHS)
      list(GET BOF3_CLAIMED_HEADER_OWNERS ${_i} _h_owner)
      if(_h_owner STREQUAL _claim_owner)
        list(APPEND source_headers "${CMAKE_SOURCE_DIR}/${_h}")
      endif()
      math(EXPR _i "${_i} + 1")
    endforeach()
  elseif(NOT group_directory STREQUAL ""
         AND NOT group_directory STREQUAL source_directory)
    # Nested lift: rebuild when any target-root header changes.
    file(GLOB_RECURSE _target_headers CONFIGURE_DEPENDS
         "${CMAKE_SOURCE_DIR}/${group_directory}/*.h")
    list(APPEND source_headers ${_target_headers})
  endif()
  if(NOT group_directory STREQUAL "")
    string(SHA1 directory_hash "${group_directory}")
    string(SUBSTRING "${directory_hash}" 0 16 directory_hash)
    list(APPEND "BOF3_TARGET_OBJECTS_${directory_hash}" "${object}")
    list(APPEND BOF3_SOURCE_DIRECTORIES "${group_directory}")
  endif()
endforeach()

list(REMOVE_DUPLICATES BOF3_SOURCE_DIRECTORIES)
foreach(source_directory IN LISTS BOF3_SOURCE_DIRECTORIES)
  string(SHA1 directory_hash "${source_directory}")
  string(SUBSTRING "${directory_hash}" 0 16 directory_hash)
  add_custom_target("target_${directory_hash}"
                    DEPENDS ${BOF3_TARGET_OBJECTS_${directory_hash}})
endforeach()

add_custom_target(lifts ALL DEPENDS ${BOF3_OBJECTS})
