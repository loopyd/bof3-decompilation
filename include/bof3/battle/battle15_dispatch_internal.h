#ifndef BOF3_BATTLE15_DISPATCH_INTERNAL_H
#define BOF3_BATTLE15_DISPATCH_INTERNAL_H

#include "bof3/battle/battle15_internal.h"

#define BATTLE_DISPATCH_WORK_TABLE(source_table) \
    do { \
        BattleSelectionDispatchTable dispatch_table; \
        u8 *dispatch_work; \
        dispatch_table = (source_table); \
        dispatch_work = g_battle_work; \
        D_801459F0 = 0x800F0800; \
        dispatch_table.handlers[dispatch_work[1]](); \
        D_801459F0 = 0x800D3800; \
    } while (0)

#endif
