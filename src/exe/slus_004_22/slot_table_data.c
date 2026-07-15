#include "internal.h"

/*
 * @behavior records reviewed shipped-file metadata for selected native EMI-loader
 * slot IDs.
 * @source D_80182444 is the native u32 LBA table; this path metadata is authored.
 */
const SlotTableEntry g_slot_table[] = {
    {EMI_LOADER_SLOT_DEMO_EMI, 60853u, "BIN/ETC/DEMO.EMI", RUNTIME_PATH_EMI},
    {EMI_LOADER_SLOT_FIRST_EMI, 61359u, "BIN/ETC/FIRST.EMI", RUNTIME_PATH_EMI},
    {EMI_LOADER_SLOT_GAME_EMI, 61520u, "BIN/ETC/GAME.EMI", RUNTIME_PATH_EMI},
    {
        EMI_LOADER_SLOT_SCENA16_EMI,
        65730u,
        "BIN/SCENARIO/SCENA16.EMI",
        RUNTIME_PATH_EMI,
    },
    {
        EMI_LOADER_SLOT_CAPCOM30_STR,
        186252u,
        "LOGO/CAPCOM30.STR",
        RUNTIME_PATH_STR,
    },
    {EMI_LOADER_SLOT_LOGO_EXE, 187407u, "LOGO/LOGO.EXE", RUNTIME_PATH_PSX_EXE},
};

const size_t g_slot_table_count = ARRAY_COUNT(g_slot_table);
