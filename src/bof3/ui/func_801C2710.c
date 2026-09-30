#include "bof3/ui/game00_internal.h"

/* @behavior Recomputes the shared view transform block from the active work
 * record: picks the base view scale from the work state byte at 0x08, rescales
 * it by the per-entity factor selected by work byte 0x11C, then commits the
 * work position advanced by the work step byte at 0x09 and copies the packed
 * facing word at 0x14 into the shared block.
 * @source 0x801C2710
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit is instruction- and byte-exact: 52/52 instructions, 208 bytes
 * (the frozen queue row's 51-instruction count was one short; the reviewed
 * Splat boundary is 0xD0 bytes = 52 instructions).
 * The scale update is written in place (`D_8014932E = D_8014932E *
 * D_80181BD4[index];`) with no `scale` temporary: the scheduler then lifts the
 * in-place multiply's load/mult above that store and gives the field value
 * `$v0`, matching the original. A named `scale` local instead keeps the
 * explicit store after the field load and allocates the field value to `$a0`
 * (first difference +0x0068; 46/52).
 */
void func_801C2710(void) {
  struct GameWorkArea* work;
  u8   mode;
  u8   index;
  s32* facing_ptr;

  mode = g_game_work->route_index_08;
  if (mode == 2 || mode == 6) {
    D_8014932E = 4;
  } else {
    D_8014932E = 8;
  }

  index = D_80146250[0x11C];
  work = g_game_work;
  facing_ptr = &work->unk_14;
  D_8014932E = D_8014932E * D_80181BD4[index];
  D_80149308 = work->coord_x_34 + work->field_0C * work->pad_09[0];
  D_8014930C = work->coord_y_38 + work->field_10 * work->pad_09[0];
  D_80149330 = *facing_ptr;
}
