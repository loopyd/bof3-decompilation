#include "internal.h"

/* @source 0x801B8234 */
/* @behavior set state byte and dispatch the current work handler */
void func_801B8234(void)
{
  D_80146250[0x12B] = 2;
  D_801CD358[((u8*)g_game_work)[3]]();
}
