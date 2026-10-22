#include "bof3/ui/game00_internal.h"

/**
 * @source 0x8019EB38
 * @behavior Initializes the active work record from the route index published
 * at the leading byte of D_80145E98: stores that byte at 0x08, then fills the
 * coordinate words at 0x34/0x38 from the D_80145EC4/D_80145EC8 bases plus the
 * doubled route-step tables, the counter halfword at 0x3E from D_80145ECE plus
 * 0xC0, the two movement words at 0x0C/0x10 from the tables at 0x80181AC0 and
 * 0x80181AC4 shifted up by four, the step byte at 0x09 with 0x0E, and clears
 * 0x0B before advancing the work substate byte at 0x01.
 * @status partial
 * @match 55.17
 * @residual scheduling residual: the 58-instruction multiset and the 232-byte
 * size are already exact, but this build fills the g_game_work pointer-load slot
 * with nop and issues the D_80145EC4 load after the route-byte load, while the
 * original issues that global load first and keeps the later statements' loads in
 * source order. Publishing the base words through locals (before or after the
 * pointer assignment) and casting the route index leave the schedule unchanged;
 * swapping the y and counter statements drops six matched instructions.
 */
void func_8019EB38(void)
{
    struct GameWorkArea *work;

    g_game_work->route_index_08 = D_80145E98[0].field_00;
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
