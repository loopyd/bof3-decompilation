bof3_define_module_artifact(
    bof3_batl_re2_01_raw
    DISC_FOLDER "BIN/BATTLE/BATL_RE2"
    PROGRAM_NAME "1.bin"
    PROGRAM_PATH "/bins/BIN/BATTLE/BATL_RE2/1.bin"
    SOURCE_HINT "build/extracted/BIN/BATTLE/BATL_RE2.EMI#1"
    DECLARED_SOURCES src/modules/batl_re2/01/func_80036e00.c
)

bof3_define_module_artifact(
    bof3_battle_03_raw
    RAW_BINARY
    DISC_FOLDER "BIN/BATTLE/BATTLE"
    PROGRAM_NAME "03.bin"
    PROGRAM_PATH "/bins/BIN/BATTLE/BATTLE/03.bin"
    SOURCE_HINT "build/extracted/BIN/BATTLE/BATTLE.EMI#3"
    LOAD_ADDRESS "0x801d0c00"
    RAW_SIZE "118224"
    SOURCE_PREFIXES "src/modules/battle/03/"
)

bof3_define_module_artifact(
    bof3_battle_15_raw
    PLACEHOLDER
    DISC_FOLDER "BIN/BATTLE/BATTLE"
    PROGRAM_NAME "15.bin"
    PROGRAM_PATH "/bins/BIN/BATTLE/BATTLE/15.bin"
    SOURCE_HINT "build/extracted/BIN/BATTLE/BATTLE.EMI#15"
    SOURCE_PREFIXES "src/modules/battle/15/"
)
