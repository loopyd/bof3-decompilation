#include "bof3/scenario/sce10eff_internal.h"
#include "gpu/prim.h"

/* @behavior transforms the current scratchpad position into screen coordinates
 * and copies the projected pair back to scratch state.
 * @source 0x801D2658
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D2658(void) {
  SVECTOR position;
  u8*     prim;

  position.vx =
      (s16)(D_1F800044->unk_34 >> 9) -
      0x4000;
  position.vy =
      (s16)(D_1F800044->unk_38 >> 9) -
      0x4000;
  position.vz =
      (s16)(-((s16)D_1F800044->unk_3e / 2));

  prim = g_PrimCursor;
  func_8017AA30(prim);
  RotTransPers(&position, (long*)(prim + 8), (long*)&position, (long*)&position.vz);

  D_1F800044->screen_x_2e =
      *(u16*)(prim + 8);
  D_1F800044->screen_y_30 =
      *(u16*)(prim + 10);
}
