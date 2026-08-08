#include "internal.h"

/**
 * @source 0x801B78B4
 * @behavior Sets the active object's state and dispatches a work-area handler.
 */
void func_801B78B4(void)
{
  D_80146250[0x12B] = 3;
  D_801CD308[((u8*)g_game_work)[3]]();
}
