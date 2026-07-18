#include "internal.h"

/* @behavior clears scratchpad work area bytes at offsets 0x00–0x04.
 * @source 0x80196070
 */
void func_80196070(void) {
    g_game_work->flags_00 = 0;
    g_game_work->unk_01 = 0;
    g_game_work->flags_02 = 0;
    g_game_work->pad_03[0] = 0;
    g_game_work->pad_03[1] = 0;
}
