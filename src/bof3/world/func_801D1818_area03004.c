#include "bof3/world/area03004_internal.h"

/* @behavior submits the AREA030 graphics-mode byte arg2 through
 * submitTpageDrawMode, then builds the 16x16 sprite primitive in place at the
 * shared primitive cursor g_PrimCursor: SetSprt16 stamps the tag, the colour
 * bytes r0/g0/b0 become the gray 0x80, the CLUT halfword becomes 0x7A40, the
 * position comes from the caller halfwords (arg0,arg1) and the texture cell
 * becomes (u0,v0) = (the 0x801E1D0C byte selected by bits 3-4 of the shared
 * frame counter, 0x48); GpuAppendPrim appends the 0x10-byte primitive under the
 * caller graphics-mode byte arg2.
 * @source 0x801D1818
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D1818(s16 arg0, s16 arg1, u8 arg2) {
  SPRT_16* primitive;

  submitTpageDrawMode(0, arg2);
  primitive = (SPRT_16*)g_PrimCursor;
  func_8017AA08(primitive);
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  primitive->clut = 0x7a40;
  primitive->x0 = arg0;
  primitive->y0 = arg1;
  primitive->u0 = D_801E1D0C[((u32)D_80143E6C >> 3) & 3];
  primitive->v0 = 0x48;
  GpuAppendPrim(arg2, 0x10);
}
