#include "bof3/ui/game00_internal.h"

/* @source 0x801B8234
 * @behavior set state byte and dispatch the current work handler
 * @status exact
 * @match 100.00
 * @residual none
 */

void publishStateByteTwo(void)
{
  D_80146250[0x12B] = 2;
  D_801CD358[((u8*)g_game_work)[3]]();
}
