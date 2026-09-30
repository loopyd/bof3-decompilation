#include "bof3/world/area03004_internal.h"

/* @behavior builds the AREA030 8x8 sprite primitive in place at the shared
 * primitive cursor g_PrimCursor: SetSprt8 stamps the tag, the colour bytes
 * r0/g0/b0 become the gray 0x80, the CLUT halfword 0x7A40 and the texture cell
 * (u0,v0) = (0xB8,0x48) select the AREA030 cell and the position comes from the
 * caller halfwords (arg0,arg1); GpuAppendPrim then appends the 0x10-byte
 * primitive under the caller graphics-mode byte arg2.
 * @source 0x801D18CC
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D18CC(s16 arg0, s16 arg1, u8 arg2) {
  SPRT_8* primitive;

  primitive = (SPRT_8*)g_PrimCursor;
  SetSprt8(primitive);
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  primitive->clut = 0x7a40;
  primitive->u0 = 0xb8;
  primitive->x0 = arg0;
  primitive->y0 = arg1;
  primitive->v0 = 0x48;
  GpuAppendPrim(arg2, 0x10);
}
