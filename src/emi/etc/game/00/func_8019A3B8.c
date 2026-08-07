#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C8090) indexed by the
 * work-area byte (work+0x01) through a tail-style jalr call.
 * @source 0x8019A3B8
 */
void func_8019A3B8(void)
{
    D_801C8090[g_game_work->unk_01]();
}
