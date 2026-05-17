#include "internal.h"

/*
 * does: holds the minimal proven slot-to-path bridge for the current title/menu
 * slice
 * vertical slice.
 * @source: 0x80182444 DAT_80182444
 * @source: processed/inventory/inventory.sqlite (slot_map)
 * @source: docs/specs/runtime/logo-boot.md
 * @source: docs/specs/runtime/first-overlay.md
 * @source: docs/specs/runtime/game-overlay.md
 */
const SlotTableEntry g_slot_table[] = {
    {SLOT_DEMO_EMI, 60853u, "BIN/ETC/DEMO.EMI", RUNTIME_PATH_EMI},
    {SLOT_FIRST_EMI, 61359u, "BIN/ETC/FIRST.EMI", RUNTIME_PATH_EMI},
    {SLOT_GAME_EMI, 61520u, "BIN/ETC/GAME.EMI", RUNTIME_PATH_EMI},
    {
     SLOT_SCENA16_EMI, 65730u,
     "BIN/SCENARIO/SCENA16.EMI", RUNTIME_PATH_EMI,
     },
    {
     SLOT_CAPCOM30_STR, 186252u,
     "LOGO/CAPCOM30.STR", RUNTIME_PATH_STR,
     },
    {SLOT_LOGO_EXE, 187407u, "LOGO/LOGO.EXE", RUNTIME_PATH_PSX_EXE},
};

const size_t g_slot_table_count = ARRAY_COUNT(g_slot_table);
