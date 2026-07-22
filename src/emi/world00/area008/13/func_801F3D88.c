#include "internal.h"

/* @behavior draws the local textured frame as four translucent FT4 border
 * strips (left edge, top/bottom span, right-inner span, right edge) around a
 * shared texture window, then queues the matching inner fill rectangle.
 * @source 0x801F3D88
 *
 * MATCHING_AID: register pins force the original's s-register allocation:
 * s1=arg0→half_width, s4=half_width, s5=arg1, s8=left_inner_x.
 * The primitive cursor is the named PsyQ global D_8014598C (not a
 * fixed-address macro) so codegen emits the symbol-relative `lui + lw` load the
 * original uses.
 */
void func_801F3D88(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4) {
  RECT*     texture_window;
  POLY_FT4* primitive;
  s16       bottom_y;
  u8        bottom_v;
  s32       clut_x;
  s32       width_plus_1;
  s32       odd_width;

  /* MATCHING_AID: pin s1 to a work register that holds arg0 first, then
   * half_width. The original loads arg0 from stack into s1 (lhu s1,24(sp)),
   * then overwrites s1 with half_width (sra s1,v0,1). */
  register s32 r_s1 asm("$17");
  /* MATCHING_AID: pin s4 to half_width. The original copies s1→s4 after
   * computing half_width (move s4,s1). */
  register s32 r_s4 asm("$20");
  /* MATCHING_AID: pin s5 to arg1. The original loads arg1 from stack into s5
   * (lhu s5,32(sp)) and keeps it there for all four primitives. */
  register s32 r_s5 asm("$21");
  /* MATCHING_AID: pin s8 to left_inner_x (arg0+2). The original computes
   * addiu s8,t1,2 and keeps s8 live across all primitives. */
  register s32 r_s8 asm("$30");

  r_s5 = (u16)arg1;
  r_s1 = (u16)arg0;
  r_s8 = (s16)(arg0 + 2);

  texture_window = (RECT*)D_8014598C;
  D_8014598C = (u8*)(texture_window + 1);
  texture_window->y = 0xf0;
  texture_window->x = 0;
  texture_window->w = 0x10;
  texture_window->h = 0x10;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  SetDrawMode((DR_MODE*)D_8014598C, 0, 1, 0xf, texture_window);
  func_8014E5A0(1u, 0x0cu);

  primitive = (POLY_FT4*)D_8014598C;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, 1);
  clut_x = ((s32)arg4 << 5) + 0x10;
  primitive->x0 = (s16)r_s1;
  primitive->y0 = (s16)r_s5;
  primitive->y1 = (s16)r_s5;
  primitive->x2 = (s16)r_s1;
  primitive->x1 = (s16)r_s8;
  primitive->x3 = (s16)r_s8;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->v1 = 0u;
  primitive->u2 = 0u;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;
  /* MATCHING_AID: keep the arg3+1 cursor-math from hoisting above the color
   * stores; the original computes bottom_v lazily here, so s2 stays arg4 (for
   * clut_x) until this point. */
  barrier();
  bottom_v = (u8)(arg3 + 1);
  bottom_y = (s16)((s32)r_s5 + bottom_v);
  primitive->y2 = bottom_y;
  primitive->y3 = bottom_y;
  primitive->u1 = 2u;
  primitive->v2 = bottom_v;
  primitive->u3 = 2u;
  primitive->v3 = bottom_v;
  primitive->y0 += 2;
  primitive->y2 -= 3;
  primitive->v0 += 2;
  primitive->v2 -= 3;
  primitive->clut = GetClut(clut_x, 0x1e1);
  width_plus_1 = (s32)(arg2 + 1);

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  primitive->tpage = 0x0fu;
  func_8014E5A0(1u, 0x28u);

  /* MATCHING_AID: use signed shift (sra) to match original. The original
   * does: lhu s1,64(sp); andi v0,s1,0xffff; addiu v0,v0,-4; sra s1,v0,1 */
  r_s1 = (s32)(u16)width_plus_1;
  r_s1 = ((s32)(u16)r_s1 - 4) >> 1;
  r_s4 = r_s1;

  primitive = (POLY_FT4*)D_8014598C;
  odd_width = (arg2 - 3) & 1;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, 1);
  primitive->x0 = (s16)r_s8;
  primitive->y0 = (s16)r_s5;
  primitive->x1 = (s16)(r_s8 + r_s1);
  primitive->y1 = (s16)r_s5;
  primitive->x2 = (s16)r_s8;
  primitive->y2 = bottom_y;
  primitive->x3 = (s16)(r_s8 + r_s1);
  primitive->y3 = bottom_y;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->u1 = (u8)r_s4;
  primitive->v1 = 0u;
  primitive->u2 = 0u;
  primitive->v2 = bottom_v;
  primitive->u3 = (u8)r_s4;
  primitive->v3 = bottom_v;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;
  primitive->clut = GetClut(clut_x, 0x1e1);

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  primitive->tpage = 0x0fu;
  func_8014E5A0(1u, 0x28u);

  primitive = (POLY_FT4*)D_8014598C;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, 1);
  primitive->y0 = (s16)r_s5;
  primitive->x0 = (s16)((s32)(u16)arg0 + r_s1 + 2);
  primitive->x1 = (s16)((s32)(u16)arg0 + r_s1 + r_s1 + odd_width + 2);
  primitive->y1 = (s16)r_s5;
  primitive->x2 = (s16)((s32)(u16)arg0 + r_s1 + 2);
  primitive->y2 = bottom_y;
  primitive->x3 = (s16)((s32)(u16)arg0 + r_s1 + r_s1 + odd_width + 2);
  primitive->y3 = bottom_y;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->u1 = (u8)(r_s4 + odd_width);
  primitive->v1 = 0u;
  primitive->u2 = 0u;
  primitive->v2 = bottom_v;
  primitive->u3 = (u8)(r_s4 + odd_width);
  primitive->v3 = bottom_v;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;
  primitive->clut = GetClut(clut_x, 0x1e1);

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  primitive->tpage = 0x0fu;
  func_8014E5A0(1u, 0x28u);

  primitive = (POLY_FT4*)D_8014598C;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, 1);
  primitive->y2 = bottom_y;
  primitive->v2 = bottom_v;
  primitive->x1 = (s16)((s32)(u16)arg0 + width_plus_1);
  primitive->y1 = (s16)r_s5;
  primitive->x3 = (s16)((s32)(u16)arg0 + width_plus_1);
  primitive->x0 = (s16)((s32)(u16)arg0 + width_plus_1 - 2);
  primitive->x2 = (s16)((s32)(u16)arg0 + width_plus_1 - 2);
  primitive->y3 = bottom_y;
  primitive->y0 = (s16)r_s5;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->u1 = 2u;
  primitive->v1 = 0u;
  primitive->u2 = 0u;
  primitive->u3 = 2u;
  primitive->v3 = bottom_v;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;
  primitive->y1 += 2;
  primitive->y3 -= 3;
  primitive->y2 -= 1;
  primitive->v1 += 2;
  primitive->v2 -= 1;
  primitive->v3 -= 3;
  primitive->clut = GetClut(clut_x, 0x1e1);

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  primitive->tpage = 0x0fu;
  func_8014E5A0(1u, 0x28u);

  texture_window = (RECT*)D_8014598C;
  D_8014598C = (u8*)(texture_window + 1);
  texture_window->x = 0;
  texture_window->y = 0;
  texture_window->w = 0x100;
  texture_window->h = 0x100;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  SetDrawMode((DR_MODE*)D_8014598C, 0, 1, 0xf, texture_window);
  func_8014E5A0(1u, 0x0cu);
  func_801AEBA0((s16)r_s8, (s16)((s32)r_s5 + 2), (s16)(arg2 - 4),
                (s16)(arg3 - 4), 0);
}
