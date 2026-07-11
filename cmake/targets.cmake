bof3_artifact_register_built(
    bof3_exe
    FOLDER ""
    PROGRAM_NAME "SLUS_004.22"
    PROGRAM_PATH "/boot/SLUS_004.22"
    SOURCE_HINT "${BOF3_EXTRACTED_DIR}/SLUS_004.22"
    KIND "boot"
    BUILT_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/bof3.exe"
    DECLARED_SOURCES ${BOF3_CORE_SOURCES}
)

bof3_artifact_register_archive(
    bof3_logo_exe
    FOLDER "LOGO"
    PROGRAM_NAME "LOGO.EXE"
    PROGRAM_PATH "/boot/LOGO/LOGO.EXE"
    SOURCE_HINT "${BOF3_EXTRACTED_DIR}/LOGO/LOGO.EXE"
    KIND "logo"
    DECLARED_SOURCES ${BOF3_MODULE_LOGO_SOURCES}
)

include("${CMAKE_CURRENT_LIST_DIR}/modules/game.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/battle.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/bate.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/commu00.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/etc.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/scenario.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/scena16.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/world00.cmake")
