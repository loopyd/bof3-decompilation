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
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${BOF3_OBJECT_FLAGS_FILE}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${BOF3_VARIANTS_FILE}")

file(GLOB_RECURSE BOF3_C_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${BOF3_C_SOURCES})
include("${CMAKE_SOURCE_DIR}/config/compiler/graph.cmake")
file(GLOB_RECURSE BOF3_ASM_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/src/*.s" "${CMAKE_SOURCE_DIR}/src/*.S")
file(GLOB_RECURSE BOF3_SHARED_HEADERS CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/include/*.h" "${CMAKE_SOURCE_DIR}/include/*.inc"
     "${CMAKE_SOURCE_DIR}/src/shared/*.inc")
file(GLOB_RECURSE BOF3_SHARED_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/src/shared/*.h" "${CMAKE_SOURCE_DIR}/src/shared/*.inc")
file(GLOB_RECURSE BOF3_COMPILER_INPUTS CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/tools/python/harness/build/*.py"
     "${CMAKE_SOURCE_DIR}/tools/python/harness/domain/*.py"
     "${CMAKE_SOURCE_DIR}/tools/python/harness/common/*.py"
     "${CMAKE_SOURCE_DIR}/third_party/maspsx/*.py"
     "${CMAKE_SOURCE_DIR}/config/targets/*/target.toml"
     "${CMAKE_SOURCE_DIR}/config/targets/*/splat.yaml"
     "${CMAKE_SOURCE_DIR}/config/targets/*/symbols.txt")
list(APPEND BOF3_COMPILER_INPUTS
     "${BOF3_CC}" "${BOF3_AS}" "${CMAKE_SOURCE_DIR}/bin/python-env"
     "${CMAKE_SOURCE_DIR}/tools/python/harness/io.py")

set(BOF3_LIFT_TARGETS)
set(BOF3_SOURCE_DIRECTORIES)

# Explicit target source claims (manifest sources/support_sources/headers):
# an authored lift may live in a semantic src/bof3/<class>/ folder far outside
# the manifest source_dir, so grouping uses the claim, never path ancestry.
# Unmigrated targets emit no claims and keep the legacy inventory grouping.
if(EXISTS "${CMAKE_SOURCE_DIR}/.venv/bin/python")
  set(BOF3_CLAIMS_PYTHON "${CMAKE_SOURCE_DIR}/.venv/bin/python")
else()
  find_program(BOF3_CLAIMS_PYTHON python3)
endif()
set(BOF3_ADAPTER_COMMAND
    "${CMAKE_COMMAND}" -E env "PYTHONPATH=${CMAKE_SOURCE_DIR}/tools/python"
    "PYTHONDONTWRITEBYTECODE=1" "${BOF3_CLAIMS_PYTHON}" -m harness.build.cli
    --root "${CMAKE_SOURCE_DIR}")
execute_process(
  COMMAND ${BOF3_ADAPTER_COMMAND} inventory
  OUTPUT_VARIABLE _inventory_json
  RESULT_VARIABLE _inventory_result
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _inventory_result EQUAL 0)
  message(FATAL_ERROR "C translation-unit inventory failed")
endif()
set(BOF3_INVENTORY_FILE "${CMAKE_CURRENT_BINARY_DIR}/bof3-inventory.json")
file(CONFIGURE OUTPUT "${BOF3_INVENTORY_FILE}" CONTENT "${_inventory_json}" @ONLY)
string(SHA256 BOF3_INVENTORY_SHA256 "${_inventory_json}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/tools/python/harness/build/inventory.py"
  "${CMAKE_SOURCE_DIR}/tools/python/harness/build/cli.py"
  "${CMAKE_SOURCE_DIR}/tools/python/harness/build/compiler.py"
  "${CMAKE_SOURCE_DIR}/tools/python/harness/domain/functions.py"
  "${CMAKE_SOURCE_DIR}/tools/python/harness/toolchain/gcc.py"
  "${CMAKE_SOURCE_DIR}/tools/python/harness/domain/tags.py"
  "${CMAKE_SOURCE_DIR}/tools/python/harness/common/lexicon.py"
  "${CMAKE_SOURCE_DIR}/tools/python/harness/domain/cache.py"
  "${CMAKE_SOURCE_DIR}/tools/python/harness/common/directory.py"
  "${CMAKE_SOURCE_DIR}/tools/python/harness/common/files.py")
add_custom_target(bof3_inventory
  COMMAND ${BOF3_ADAPTER_COMMAND} inventory --snapshot "${BOF3_INVENTORY_FILE}"
          --expected-sha256 "${BOF3_INVENTORY_SHA256}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" VERBATIM)

include("${BOF3_OBJECT_FLAGS_FILE}" OPTIONAL)

# Target metadata roots: every manifest's source_dir.
# Nested subsystem folders below a root share that root's build target so
# `bin/harness build TARGET` compiles every descendant lift.
set(BOF3_MANIFEST_SOURCE_DIRS)
file(GLOB_RECURSE BOF3_MANIFEST_TOMLS CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/config/targets/*.toml")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${BOF3_MANIFEST_TOMLS})
foreach(manifest_file IN LISTS BOF3_MANIFEST_TOMLS)
  file(READ "${manifest_file}" _manifest_text)
  if(_manifest_text MATCHES "source_dir[ \t]*=[ \t]*\"([^\"]+)\"")
    list(APPEND BOF3_MANIFEST_SOURCE_DIRS "${CMAKE_MATCH_1}")
  endif()
endforeach()
list(REMOVE_DUPLICATES BOF3_MANIFEST_SOURCE_DIRS)

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
  set(grouped FALSE)
  if(extension STREQUAL ".c")
    string(JSON grouped GET "${_inventory_json}" sources "src/${relative}")
    string(JSON _flags_type TYPE "${_inventory_json}" profiles "src/${relative}" flags)
    if(_flags_type STREQUAL "ARRAY")
      set(_object_flags)
      string(JSON _flags_count LENGTH "${_inventory_json}" profiles "src/${relative}" flags)
      if(_flags_count GREATER 0)
        math(EXPR _flags_last "${_flags_count} - 1")
        foreach(_flag_index RANGE ${_flags_last})
          string(JSON _flag GET "${_inventory_json}" profiles "src/${relative}" flags ${_flag_index})
          list(APPEND _object_flags "${_flag}")
        endforeach()
      endif()
      set(object_cflags ${BOF3_CFLAGS_BASE} ${_object_flags})
    else()
      set(object_cflags ${BOF3_CFLAGS})
    endif()
    # Per-object compiler variant override — wraps bin/cc with cmake -E env.
    string(JSON _compiler_type TYPE "${_inventory_json}" profiles "src/${relative}" compiler_id)
    if(_compiler_type STREQUAL "STRING")
      string(JSON _variant_id GET "${_inventory_json}" profiles "src/${relative}" compiler_id)
      execute_process(
        COMMAND "${CMAKE_SOURCE_DIR}/bin/harness" "build" "variants" "path" "${_variant_id}"
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
    if(grouped)
      set(_cc_command ${BOF3_ADAPTER_COMMAND} producer --)
      if(_compiler_type STREQUAL "STRING")
        list(PREPEND _cc_command "${CMAKE_COMMAND}" -E env "PSX_GCC=${_variant_gcc_path}")
      endif()
    endif()
  endif()

  # The public bin/harness build command resolves friendly function IDs to this stable
  # internal target; callers never need to know the encoded target name.
  string(SHA1 target_hash "src/${relative}")
  string(SUBSTRING "${target_hash}" 0 16 target_hash)
  set(lift_target "lift_${target_hash}")
  list(APPEND BOF3_LIFT_TARGETS "${lift_target}")

  # Group every source under the deepest owning manifest source_dir so a
  # relocated (nested) lift still builds with `bin/harness build TARGET`.  An explicit
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
        # target but give them no target group (no bin/harness build TARGET claim).
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
    list(APPEND "BOF3_TARGET_OBJECTS_${directory_hash}" "${lift_target}")
    list(APPEND BOF3_SOURCE_DIRECTORIES "${group_directory}")
  endif()
  if(extension STREQUAL ".c")
    bof3_add_lift("${lift_target}" "${source}" "${object}" "${grouped}"
      COMMAND ${_cc_command} ${BOF3_CPPFLAGS} ${object_cflags} ${BOF3_CC_ASFLAGS}
              -c "${source}" -o "${object}"
      DEPENDS ${source_headers} ${BOF3_SHARED_HEADERS}
              ${BOF3_SHARED_SOURCES} ${BOF3_COMPILER_INPUTS})
  else()
    bof3_add_lift("${lift_target}" "${source}" "${object}" FALSE
      COMMAND "${BOF3_AS}" ${BOF3_ASFLAGS} -o "${object}" "${source}")
  endif()
endforeach()

list(REMOVE_DUPLICATES BOF3_SOURCE_DIRECTORIES)
foreach(source_directory IN LISTS BOF3_SOURCE_DIRECTORIES)
  string(SHA1 directory_hash "${source_directory}")
  string(SUBSTRING "${directory_hash}" 0 16 directory_hash)
  add_custom_target("target_${directory_hash}"
                    DEPENDS ${BOF3_TARGET_OBJECTS_${directory_hash}})
endforeach()

add_custom_target(lifts ALL DEPENDS ${BOF3_LIFT_TARGETS})

execute_process(
  COMMAND ${BOF3_ADAPTER_COMMAND} inventory --snapshot "${BOF3_INVENTORY_FILE}"
          --expected-sha256 "${BOF3_INVENTORY_SHA256}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  RESULT_VARIABLE _inventory_check_result
  ERROR_VARIABLE _inventory_check_error)
if(NOT _inventory_check_result EQUAL 0)
  message(FATAL_ERROR "Build inventory changed during configure: ${_inventory_check_error}")
endif()
