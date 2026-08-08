#include "internal.h"

/*
 * @behavior Dispatches the mode handler table (0x801C88FC) indexed by the
 * work-area mode byte (work+0x04) through a tail-style jalr call.
 * @source 0x801AC9D0
 */
void func_801AC9D0(void)
{
    D_801C88FC[g_game_work->field_04]();
}
