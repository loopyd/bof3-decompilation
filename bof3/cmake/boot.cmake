bof3_collect_sources_with_prefix(
    BOF3_BOOT_DECLARED_SOURCES
    "src/boot/"
    "src/core/disc/"
    "src/core/callback_scheduler/"
    "src/core/game_front/"
    "src/core/emi/"
)

bof3_artifact_register_built(
    bof3_exe
    FOLDER ""
    PROGRAM_NAME "SLUS_004.22"
    PROGRAM_PATH "/boot/SLUS_004.22"
    SOURCE_HINT "build/extracted/SLUS_004.22"
    KIND "boot"
    BUILT_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/bof3.exe"
    DECLARED_SOURCES ${BOF3_BOOT_DECLARED_SOURCES}
)
