#include "bof3/ui/game00_internal.h"

/*
 * @behavior Dispatches the handler table (0x801C8904) indexed by the
 * work-area mode byte (work+0x04) through a tail-style jalr call.
 * @source 0x801ACBC4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801ACBC4(void)
{
    D_801C8904[g_game_work->field_04]();
}
