#include "internal.h"

/**
 * @source 0x801B9048
 * @behavior Set the active work mode and dispatch the handler selected by the
 * current game-work state.
 */
void func_801B9048(void)
{
  D_80146250[0x12B] = 6;
  D_801CD374[g_game_work->pad_03]();
}
