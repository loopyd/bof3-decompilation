#include "bof3/ui/game00_internal.h"

/*
 * @behavior Dispatches the handler table (0x801C8090) indexed by the
 * work-area byte (work+0x01) through a tail-style jalr call.
 * @source 0x8019A3B8
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8019A3B8(void)
{
    D_801C8090[g_game_work->unk_01]();
}
