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

  get_filename_component(source_directory "src/${relative}" DIRECTORY)
  string(SHA1 directory_hash "${source_directory}")
  string(SUBSTRING "${directory_hash}" 0 16 directory_hash)
  list(APPEND "BOF3_TARGET_OBJECTS_${directory_hash}" "${object}")
  list(APPEND BOF3_SOURCE_DIRECTORIES "${source_directory}")
endforeach()

list(REMOVE_DUPLICATES BOF3_SOURCE_DIRECTORIES)
foreach(source_directory IN LISTS BOF3_SOURCE_DIRECTORIES)
  string(SHA1 directory_hash "${source_directory}")
  string(SUBSTRING "${directory_hash}" 0 16 directory_hash)
  add_custom_target("target_${directory_hash}"
                    DEPENDS ${BOF3_TARGET_OBJECTS_${directory_hash}})
endforeach()

add_custom_target(lifts ALL DEPENDS ${BOF3_OBJECTS})
