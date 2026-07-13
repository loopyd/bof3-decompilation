find_program(HARNESS_CARGO_EXECUTABLE cargo)

if(HARNESS_CARGO_EXECUTABLE)
    function(harness_add_rust_tool target crate output_name)
        set(target_dir "${CMAKE_BINARY_DIR}/tools/rust/${output_name}")
        add_custom_target(
            "${target}"
            COMMAND
                "${HARNESS_CARGO_EXECUTABLE}" build --locked --release
                --manifest-path "${CMAKE_SOURCE_DIR}/third_party/${crate}/Cargo.toml"
                --target-dir "${target_dir}"
            BYPRODUCTS "${target_dir}/release/${output_name}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Building canonical Rust tool ${output_name}"
            VERBATIM
        )
    endfunction()

    harness_add_rust_tool(bof3_disk_tool bof3-disk-v2 bof3-disk)
    harness_add_rust_tool(emi_ex_tool emi-ex-v2 emi-ex)
    add_custom_target(harness_tools)
    add_dependencies(harness_tools bof3_disk_tool emi_ex_tool)
else()
    message(STATUS "cargo not found; canonical Rust extraction tools are unavailable")
endif()
