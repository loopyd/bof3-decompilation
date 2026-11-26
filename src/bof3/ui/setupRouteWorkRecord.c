#include "bof3/ui/game00_internal.h"

/**
 * @source 0x8019EB38
 * @behavior Initializes the active work record from the route index published
 * at the leading byte of D_80145E98: stores that byte at 0x08, then fills the
 * coordinate words at 0x34/0x38 from the D_80145EC4/D_80145EC8 bases plus the
 * doubled route-step tables, the counter halfword at 0x3E from D_80145ECE plus
 * 0xC0, the two movement words at 0x0C/0x10 from the tables at 0x80181AC0 and
 * 0x80181AC4 shifted up by four, the step byte at 0x0A with 0x0E, and clears
 * 0x0B before advancing the work substate byte at 0x01.
 * @status partial
 * @match 91.53
 * @residual 54/58 original instructions match, with 236 current bytes versus
 * 232 original. The coordinate pointer-load delay and coordinate load/store
 * scheduling still differ, including an extra NOP. Byte comparison differs.
 */
void setupRouteWorkRecord(void)
{
    struct GameWorkArea *work;
    struct GameWorkArea *tail;
    s32 movement_y;
    s32 movement_x;
    s32 counter_value;
    s32 *movement_source;

    g_game_work->route_index_08 = D_80145E98[0].field_00;
    work = g_game_work;
    do {
        work->coord_x_34 = D_80145EC4 + D_80181B94[work->route_index_08 * 2];
        work->coord_y_38 = D_80145EC8 + D_80181B98[work->route_index_08 * 2];
    } while (0);
    counter_value = D_80145ECE + 0xC0;
    movement_source = &D_80181AC0[work->route_index_08 * 2];
    do {
        work->counter_3E = counter_value;
    } while (0);
    movement_x = *movement_source << 4;
    movement_source = &D_80181AC4[work->route_index_08 * 2];
    do {
        work->field_0C = movement_x;
    } while (0);
    movement_y = *movement_source;
    do {
        work->pad_09[1] = 0xE;
    } while (0);
    tail = g_game_work;
    work->field_10 = movement_y << 4;
    tail->field_0B = 0;
    g_game_work->unk_01++;
}
