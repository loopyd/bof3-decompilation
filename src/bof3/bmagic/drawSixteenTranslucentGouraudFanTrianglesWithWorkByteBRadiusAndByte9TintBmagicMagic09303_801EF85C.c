#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */
extern s32 GetGraphType(void);
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */

/* @source 0x801EF85C
 * @behavior Emit sixteen semi-transparent Gouraud triangles around the origin.
 *           Select draw-mode page 0xA5 for graph types 1 and 2, otherwise 0x35,
 *           and submit its 0xC-byte packet using work words 0x34 and 0x38,
 *           kind 2. Publish radius 0x100 plus the phase sampler of
 *           (work byte B & 0x1F) << 7, shifted left four then right twelve,
 *           in scratchpad halfword 0. Read work byte 9 once for tint weights:
 *           (10,9,2) at the origin and (10,4,2) at both rim vertices.
 *           Seed the prior point at phase zero. Each pass advances phase by
 *           0x100, carries the previous point, samples the new point, projects
 *           the three vertices with RotTransPers3 and submits a 0x1C-byte packet
 *           with the same reloaded work coordinates and kind 2. All three z
 *           halfwords are zero. Takes no arguments and returns nothing.
 *           Work field roles beyond these accesses remain offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSixteenTranslucentGouraudFanTrianglesWithWorkByteBRadiusAndByte9TintBmagicMagic09303_801EF85C(void) {
  u16 *cells;
  u16 *radius;
  u16 *next;
  SVECTOR *shape;
  POLY_G3 *prim;
  long depth;
  long flag;
  s32 graph;
  s32 shade;
  s32 red;
  s32 green;
  s32 blue;
  s32 rim_green;
  s32 phase;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34),
                *(s32 *)(g_battle_work + 0x38), 2, 0xC);

  cells = &D_1F800000;
  *cells = ((func_801782FC((g_battle_work[0xB] & 0x1F) << 7) << 4) >> 12) + 0x100;
  shade = g_battle_work[9];
  red = shade * 10;
  green = shade * 9;
  blue = shade * 2;
  rim_green = shade * 4;

  next = &D_1F800024;
  *next = (func_801782FC(0) * *(s16 *)cells) >> 12;
  SPAD_REF(u16, 0x26u) = (func_801783C8(0) * *(s16 *)cells) >> 12;
  SPAD_REF(u16, 0x28u) = 0;
  SPAD_REF(u16, 0x20u) = 0;
  SPAD_REF(u16, 0x18u) = 0;

  i = 1;
  shape = (SVECTOR *)(next - 8);
  radius = cells;
  do {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);

    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;

    phase = i << 8;
    shape[2].vx = (func_801782FC(phase) * *(s16 *)radius) >> 12;
    shape[2].vy = (func_801783C8(phase) * *(s16 *)radius) >> 12;
    RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);

    prim->r0 = red;
    prim->g0 = green;
    prim->b0 = blue;
    prim->r1 = red;
    prim->g1 = rim_green;
    prim->b1 = blue;
    prim->r2 = red;
    prim->g2 = rim_green;
    prim->b2 = blue;
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0x1C);
    i++;
  } while (i < 0x11);
}
