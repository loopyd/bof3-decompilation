#include "bof3/ui/game00_internal.h"

/**
 * @source 0x8019EB38
 * @behavior Initializes work-area route and coordinates from global tables,
 * then advances the work substate.
 */
void func_8019EB38(void)
{
    struct GameWorkArea *work;

    g_game_work->route_index_08 = D_80145E98;
    work = g_game_work;
    work->coord_x_34 = D_80145EC4 + D_80181B94[work->route_index_08 * 2];
    work->coord_y_38 = D_80145EC8 + D_80181B98[work->route_index_08 * 2];
    work->counter_3E = D_80145ECE + 0xC0;
    work->field_0C = D_80181AC0[work->route_index_08 * 2] << 4;
    work->pad_09[1] = 0xE;
    work->field_10 = D_80181AC4[work->route_index_08 * 2] << 4;
    g_game_work->field_0B = 0;
    g_game_work->unk_01++;
}
