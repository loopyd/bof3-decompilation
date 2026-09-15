function(bof3_add_lift target source object grouped)
  cmake_parse_arguments(LIFT "" "" "COMMAND;DEPENDS" ${ARGN})
  get_filename_component(object_directory "${object}" DIRECTORY)
  if(grouped)
    add_custom_target("${target}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${object_directory}"
      COMMAND ${LIFT_COMMAND}
      DEPENDS "${source}" ${LIFT_DEPENDS} bof3_inventory
      BYPRODUCTS "${object}" "${object}.s" "${object}.producer.json"
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      VERBATIM)
  else()
    add_custom_command(
      OUTPUT "${object}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${object_directory}"
      COMMAND ${LIFT_COMMAND}
      DEPENDS "${source}" ${LIFT_DEPENDS} bof3_inventory "${BOF3_INVENTORY_FILE}"
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      VERBATIM)
    add_custom_target("${target}" DEPENDS "${object}")
    add_dependencies("${target}" bof3_inventory)
  endif()
endfunction()
