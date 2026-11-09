#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80143BB0;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);

/* @source 0x801F9354
 * @behavior Word 5 of the seventeen-word in-image primary state table at
 * 0x801FE3EC, the word stored at 0x801FE400, that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets 0x801F9354 and that one table word is
 * its only reference in this payload. It reads the shared sub-state byte
 * D_80146875 once through one address taken into a local pointer, which keeps
 * that base in $s1 for the whole body, and it runs two alternative sequences
 * over that byte. Sub-state 0, while the unsigned mode byte D_80143BB0
 * (0x80143BB0) is not 2, sets the shared scenario progress byte
 * g_ScenarioProgress (0x80146864) to 1, runs the shared helper
 * func_801BE1B0(0xC) and advances the sub-state byte to 1. Sub-state 1, once the
 * progress byte reads 9, sets scenario flag bit 0x3F of the shared flag word
 * D_8014686C (0x8014686C, read with lw) through func_8015B580, runs the shared
 * release helper func_8015C058 and jumps to the shared clearing tail. Sub-state
 * 5, under the same not-2 mode gate, sets the progress byte to 1 and advances the
 * sub-state byte to 6. Sub-state 6, once the progress byte reads 2, runs the
 * release helper, clears the progress byte and falls out of the switch into that
 * same clearing tail, which clears the shared bytes D_80146865, D_80146866 and
 * D_80146867, the sub-state byte and the primary state byte D_80146874 back to
 * zero, which ends the state. Every other sub-state value returns without
 * touching that tail. It takes no arguments, reads no other state and writes no
 * other byte, and its 0x20-byte frame keeps $ra plus the two saved registers: $s0
 * holds the constant 1 across the func_801BE1B0 call in sub-state 0 and the
 * g_ScenarioProgress address across the func_8015C058 call in sub-state 6, which
 * is why that address is taken through a block-local pointer there while
 * sub-states 0 and 5 materialise the 0x8014 page in $at for their single progress
 * stores. The four case bodies follow the sub-state order 0, 1, 5, 6, the
 * constant 2 of the sub-state 5 mode test is left in $v0 for the sub-state 6
 * progress test, and the clearing tail sits after the switch.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armProgress1UnlessMode2ThenWaitProgress9SetFlag3FOrProgress2AndClearStateScenarioScena0900_801F9354(void) {
    u8* state = &D_80146875;

    switch (*state) {
    case 0:
        if (D_80143BB0 == 2u) {
            return;
        }
        g_ScenarioProgress = 1u;
        func_801BE1B0(0xcu);
        *state = 1u;
        return;

    case 1:
        if (g_ScenarioProgress != 9u) {
            return;
        }
        func_8015B580(D_8014686C, 0x3f);
        func_8015C058();
        goto clear;

    case 5:
        if (D_80143BB0 == 2u) {
            return;
        }
        g_ScenarioProgress = 1u;
        *state = 6u;
        return;

    case 6: {
        u8* progress = &g_ScenarioProgress;

        if (*progress != 2u) {
            return;
        }
        func_8015C058();
        *progress = 0u;
        break;
    }

    default:
        return;
    }

clear:
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    *state = 0u;
    D_80146874 = 0u;
}
