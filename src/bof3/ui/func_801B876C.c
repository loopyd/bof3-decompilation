#include "bof3/ui/game00_internal.h"

/* @behavior Resets local route state and selects mode 4 on shared work. */
/* @source 0x801B876C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801B876C(void)
{
    g_game_work->pad_09[0] = 0;
    g_game_work->field_0B = g_game_work->route_index_08;
    D_80146250[0x11C] = 4;
    g_game_work->pad_03 = 1;
}
