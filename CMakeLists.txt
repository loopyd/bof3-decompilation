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
    add_custom_command(
      OUTPUT "${object}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${object_directory}"
      COMMAND "${BOF3_CC}" ${BOF3_CPPFLAGS} ${BOF3_CFLAGS} ${BOF3_CC_ASFLAGS}
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
