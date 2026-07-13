harness_artifact_register_archive(
    slus_004_22
    FOLDER ""
    PROGRAM_NAME "SLUS_004.22"
    PROGRAM_PATH "/boot/SLUS_004.22"
    SOURCE_HINT "${HARNESS_EXTRACTED_DIR}/SLUS_004.22"
    KIND "boot"
    DECLARED_SOURCES ${HARNESS_CORE_SOURCES}
)

harness_artifact_register_archive(
    logo
    FOLDER "LOGO"
    PROGRAM_NAME "LOGO.EXE"
    PROGRAM_PATH "/boot/LOGO/LOGO.EXE"
    SOURCE_HINT "${HARNESS_EXTRACTED_DIR}/LOGO/LOGO.EXE"
    KIND "logo"
    DECLARED_SOURCES ${HARNESS_TARGET_EXE_LOGO_SOURCES}
)

include("${CMAKE_CURRENT_LIST_DIR}/modules/game.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/battle.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/bate.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/commu00.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/etc.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/scenario.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/scena16.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modules/world00.cmake")
