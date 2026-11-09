#include "bof3/scenario/scena00_internal.h"

extern u8 D_8014933B; /* @source 0x8014933B @kind data */

void func_801C16B8(void);

/* @behavior Entry 5 of the overlay progress-handler table at D_801FE970 that
 * dispatchProgressHandlerScenarioScena0800_801FAB5C indexes with the signed
 * byte D_80146874: it drives the shared sub-state byte D_80146875 through six
 * steps. Sub-state 0 seeds the front selector context through
 * func_8019FA28(2, 0x4E0000, 0x340000, 0x85), stores 0xFF in D_80143F1F and
 * advances to 0x64. Sub-state 0x64 waits for the shared halfword D_80143F00 to
 * read 2, then asks func_8019601C for an object index and, unless that index
 * reads 0xFF, seeds that object row of the 0x74-byte table at D_80143FC8
 * (+0 = 1, +5 = 0x57) before advancing to 1. Sub-state 1, at progress byte 1,
 * seeds the front selector context func_8019FA28(0x35, 0x3D8000, 0xD8000,
 * 0x83), stores 0xFF in D_80143F1D and D_80143F1F and advances to 2, then falls
 * through to sub-state 2, which at progress byte 2 seeds
 * func_8019FA28(0x35, 0x138000, 0x180000, 0x84) and advances to 3. Sub-state 3,
 * at progress byte 6, raises the shared byte D_8014933B to 1 and advances to 4.
 * Sub-state 4, at progress byte 8, runs func_801C16B8, func_8015B580 with the
 * shared word D_8014686C and 7, func_8019FA28(0x73, 0x1B0000, 0x210000, 3) and
 * func_8015C058, clears flag bit 3 of the mode halfword D_80146258, and returns
 * the overlay to handler 0 by clearing the progress byte at 0x80146864,
 * D_80146874 and the sub-state byte. Any other sub-state value does nothing. It
 * takes no arguments, returns nothing, and its 0x20-byte frame keeps $ra plus
 * the two saved registers that hold the sub-state address and the address of
 * the progress byte.
 * @source 0x801FBA70
 * @status partial
 * @match 90.07
 * @residual Saved-register pair exchanged: the built body keeps the sub-state
 * address in $s0 and the progress-byte address in $s1 where the original holds
 * the sub-state address in $s1 and the progress-byte address in $s0, so 14 of
 * 141 instructions differ at unchanged size (564 bytes) and frame (0x20) with
 * the same save group. Seven clean-C variants measured through the sweeper:
 * three neutral (switch-value local, case-local progress pointer, both
 * together), two regressions that folded the case-4 address and lost the save
 * (function-scope progress pointer declared before or after the sub-state
 * pointer, 552 bytes), and two neutral separate-spelling variants
 * (0x80146864/0x80146875 forms of the same cell). The allocator margin is
 * measured: the sub-state pointer is 7 references across 91 live insns
 * (priority 1538) and the case-4 progress address 3 references across 20 live
 * insns (priority 1500), so the exchange needs 14/live_state < 3/live_progress,
 * i.e. 19 or fewer live insns for the progress address or 94 or more for the
 * sub-state pointer; every shape tried holds 91/20.
 */
void gateProgressStepsRequestingFrontSelectorContextsThenResetHandlerScenarioScena0800_801FBA70(void) {
    u8 *state;

    state = &D_80146875;
    switch (*state) {
    case 0:
        func_8019FA28(2u, 0x4e0000u, 0x340000u, 0x85u);
        D_80143F1F = 0xffu;
        *state = 0x64u;
        break;

    case 0x64: {
        u8 *row;
        u8 object_index;

        if (D_80143F00 != 2u) {
            break;
        }
        object_index = func_8019601C();

        if (object_index != 0xffu) {
            row = &D_80143FC8[(u32)object_index * 0x74u];
            row[0] = 1u;
            row[5] = 0x57u;
        }
        *state = 1u;
        break;
    }

    case 1:
        if (*(u8 *)&g_ScenarioProgress == 1u) {
            func_8019FA28(0x35u, 0x3d8000u, 0xd8000u, 0x83u);
            D_80143F1D = 0xffu;
            D_80143F1F = 0xffu;
            *state = 2u;
        }
        /* Falls through. */
    case 2:
        if (*(u8 *)&g_ScenarioProgress == 2u) {
            func_8019FA28(0x35u, 0x138000u, 0x180000u, 0x84u);
            D_80146875 = 3u;
        }
        break;

    case 3:
        if (*(u8 *)&g_ScenarioProgress == 6u) {
            D_8014933B = 1u;
            *state = 4u;
        }
        break;

    case 4:
        if (*(u8 *)&g_ScenarioProgress != 8u) {
            break;
        }
        func_801C16B8();
        func_8015B580(D_8014686C, 7);
        func_8019FA28(0x73u, 0x1b0000u, 0x210000u, 3u);
        D_80146258 = (u16)(D_80146258 & 0xfff7u);
        func_8015C058();
        *(u8 *)&g_ScenarioProgress = 0;
        D_80146874 = 0;
        *state = 0u;
        break;
    }
}
