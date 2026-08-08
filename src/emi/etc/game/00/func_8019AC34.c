#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C80EC) indexed by the
 * work-area byte (work+0x01), then sets the shared state byte to 2.
 * @source 0x8019AC34
 */
void NO_SIBLING_CALLS func_8019AC34(void)
{
    D_801C80EC[g_game_work->unk_01]();
    D_80149333 = 2;
}
