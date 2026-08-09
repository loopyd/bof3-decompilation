#include "bof3/ui/game00_internal.h"

/* @source 0x801B8234
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
/* @behavior set state byte and dispatch the current work handler */
void func_801B8234(void)
{
  D_80146250[0x12B] = 2;
  D_801CD358[((u8*)g_game_work)[3]]();
}
