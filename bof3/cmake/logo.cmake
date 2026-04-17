bof3_collect_sources_with_prefix(
    BOF3_LOGO_DECLARED_SOURCES
    "src/modules/logo/"
)

bof3_artifact_register_archive(
    bof3_logo_exe_raw
    FOLDER "LOGO"
    PROGRAM_NAME "LOGO.EXE"
    PROGRAM_PATH "/boot/LOGO/LOGO.EXE"
    SOURCE_HINT "build/extracted/LOGO/LOGO.EXE"
    KIND "logo"
    DECLARED_SOURCES ${BOF3_LOGO_DECLARED_SOURCES}
)
