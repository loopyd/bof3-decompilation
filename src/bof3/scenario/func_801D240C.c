#include "bof3/scenario/sce10eff_internal.h"
#include "gpu/prim.h"
#include <libgpu.h>
#include <stdlib.h>

/* @behavior builds eight translucent quadrilateral primitives around the
 * current scratchpad screen position from adjacent angular samples, reusing
 * each iteration's second sample as the next iteration's first sample.
 * @source 0x801D240C
 * @status partial
 * @match 82.31
 * @residual size and loop-local lifetime scheduling differ
 */
void func_801D240C(void) {
  u8* primitive;
  s32 phase;
  s32 next_phase;
  s32 graph_value;
  s32 i;

  SPAD_REF(s32, 0) = (rand() & 7) + D_1F800044->color_0a * 3;

  if (GetGraphType() == 1) {
    graph_value = 0xa5;
  } else if (GetGraphType() == 2) {
    graph_value = 0xa5;
  } else {
    graph_value = 0x35;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, graph_value, 0);
  func_8014E5A0(2, 12);

  i = 0;
  do {
    primitive = g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);

    *(u16*)(primitive + 8) = D_1F800044->screen_x_2e;
    *(u16*)(primitive + 10) = D_1F800044->screen_y_30;

    phase = i << 9;
    *(u16*)(primitive + 16) = D_1F800044->screen_x_2e +
        ((func_801782FC(phase) * SPAD_REF(s32, 0)) >> 12);
    *(u16*)(primitive + 18) = D_1F800044->screen_y_30 +
        ((func_801783C8(phase) * SPAD_REF(s32, 0)) >> 12);

    i++;
    next_phase = i << 9;
    *(u16*)(primitive + 24) = D_1F800044->screen_x_2e +
        ((func_801782FC(next_phase) * SPAD_REF(s32, 0)) >> 12);
    *(u16*)(primitive + 26) = D_1F800044->screen_y_30 +
        ((func_801783C8(next_phase) * SPAD_REF(s32, 0)) >> 12);

    primitive[4] = D_1F800044->color_0a << 3;
    primitive[5] = D_1F800044->color_0a << 3;
    primitive[12] = 1;
    primitive[13] = 1;
    primitive[14] = 1;
    primitive[20] = 1;
    primitive[21] = 1;
    primitive[22] = 1;
    primitive[6] = D_1F800044->color_0a * 6;
    func_8014E5A0(2, 28);
  } while (i < 8);
}
