#include "internal.h"

/* @source 0x8019F9D0 */
/* @behavior Copies the route state into the active state and clears transition fields. */
void func_8019F9D0(void)
{
    g_game_work->unk_01 = g_game_work->field_0B;
    g_game_work->flags_02 = 0;
    g_game_work->pad_03 = 0;
    g_game_work->field_04 = 0;
    g_game_work->pad_09[0] = 0;
}
