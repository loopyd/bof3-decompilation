#include "bof3/bof3.h"
#include "gpu/prim.h"

extern s32 func_801782FC(s32 angle); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 @kind function */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 @kind function */
extern u8 D_801F4320[]; /* @source 0x801F4320 @kind data */
extern u8 D_801F4321[]; /* @source 0x801F4321 @kind data */
extern s16 D_1F800034[]; /* @source 0x1F800034 @kind unknown */
extern s16 D_1F800036; /* @source 0x1F800036 @kind unknown */
extern s16 D_1F800038; /* @source 0x1F800038 @kind unknown */
extern u16 D_1F800014[]; /* @source 0x1F800014 @kind unknown */
extern u16 D_1F800016[]; /* @source 0x1F800016 @kind unknown */
extern u16 D_1F800018[]; /* @source 0x1F800018 @kind unknown */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F3584
 * @behavior Draws 32 textured quadrilateral strips between two rings with the
 * caller's inner and outer radii. Builds four scratchpad vertices per strip
 * around (-0x3A40, -0x2F40, -0x3C0), projects them, sets color 0x50 and CLUT
 * 0x7940, and derives UVs from the radii and the selected byte pair at
 * 0x801F4320. Links each 0x28-byte packet using the current work object's
 * words at offsets 0x34 and 0x38. The third argument selects the texture pair
 * and page; the page encoding depends on GetGraphType.
 * @status partial
 * @match 54.86
 * @residual Frame, scratchpad load widths, page-expression placement and register allocation differ.
 */
void draw32TexturedRingStripsWorld03Area14313_801F3584(s32 inner, s32 outer,
                                                    s32 texture) {
  s32 step;
  s32 i;
  s32 offset;
  s32 angle;
  POLY_FT4 *quad;
  s32 pair;
  s16 *center;
  s32 p;
  s32 flag;

  step = 0;
  center = D_1F800034;
  pair = texture * 2;
  center[0] = -0x3A40;
  D_1F800036 = -0x2F40;
  D_1F800038 = -0x3C0;
  i = 0;
  do {
    quad = (POLY_FT4 *)g_PrimCursor;
    offset = 0;
    SetPolyFT4(quad);
    SetShadeTex(quad, 0);
    do {
      angle = (step + i) << 7;
      *(u16 *)((u8 *)D_1F800014 + offset) = center[0];
      *(u16 *)((u8 *)D_1F800016 + offset) = center[1] -
          ((func_801782FC(angle) * inner) >> 13);
      *(u16 *)((u8 *)D_1F800018 + offset) = center[2] +
          ((func_801783C8(angle) * inner) >> 13);
      i++;
      offset += 8;
    } while (i < 2);
    i = 0;
    offset = 0x10;
    do {
      angle = (step + i) << 7;
      *(u16 *)((u8 *)D_1F800014 + offset) = center[0];
      *(u16 *)((u8 *)D_1F800016 + offset) = center[1] -
          ((func_801782FC(angle) * outer) >> 13);
      *(u16 *)((u8 *)D_1F800018 + offset) = center[2] +
          ((func_801783C8(angle) * outer) >> 13);
      i++;
      offset += 8;
    } while (i < 2);
    RotTransPers4((SVECTOR *)D_1F800014, (SVECTOR *)(D_1F800014 + 4),
                 (SVECTOR *)(D_1F800014 + 8), (SVECTOR *)(D_1F800014 + 12),
                 (s32 *)&quad->x0, (s32 *)&quad->x1,
                 (s32 *)&quad->x2, (s32 *)&quad->x3, &p, &flag);
    SetSemiTrans(quad, 0);
    quad->r0 = quad->g0 = quad->b0 = 0x50;
    quad->clut = 0x7940;
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      quad->tpage = (((texture * 128 + 0x140) & 0x3FF) >> 6) | 0x220;
    } else {
      quad->tpage = (((texture * 128 + 0x140) & 0x3FF) >> 6) | 0x90;
    }
    angle = step << 7;
    quad->u0 = D_801F4320[pair] + ((func_801782FC(angle) * inner) >> 16);
    quad->v0 = D_801F4321[pair] + ((func_801783C8(angle) * inner) >> 16);
    i = step + 1;
    offset = i << 7;
    quad->u1 = D_801F4320[pair] + ((func_801782FC(offset) * inner) >> 16);
    quad->v1 = D_801F4321[pair] + ((func_801783C8(offset) * inner) >> 16);
    quad->u2 = D_801F4320[pair] + ((func_801782FC(angle) * outer) >> 16);
    quad->v2 = D_801F4321[pair] + ((func_801783C8(angle) * outer) >> 16);
    quad->u3 = D_801F4320[pair] + ((func_801782FC(offset) * outer) >> 16);
    quad->v3 = D_801F4321[pair] + ((func_801783C8(offset) * outer) >> 16);
    step = i;
    func_80155A08(FIELD_REF(s32, D_1F800044, 0x34u),
                 FIELD_REF(s32, D_1F800044, 0x38u), 0, 0x28);
    i = 0;
  } while (step < 0x20);
}
