#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800014; /* @source 0x1F800014 @kind data */
extern void func_8017A97C(void *arg0); /* @source 0x8017A97C */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
s32 GetGraphType(void);

/* @source 0x801EF294
 * @behavior Draws eight semi-transparent gouraud triangles as a closed fan
 *           from the origin to a radius-384 ring, advancing phase by 0x200.
 *           The caller at 0x801EEF48 has installed the work-record transform
 *           and follows this call with PopMatrix. Takes no arguments and
 *           returns nothing. Seeds the radius at scratchpad 0x1F800000 and
 *           the phase-zero point at 0x1F800024 and 0x1F800026. Tint at
 *           0x1F80000E is work byte 9 times eight, capped at 128; vertex 0
 *           takes that tint and both rim vertices take RGB (1,1,1).
 *           Copies the prior ring point before sampling each new point,
 *           clears all three vertex z values, projects with RotTransPers3,
 *           and appends each 0x1C-byte primitive to ordering-table index 5.
 *           Brackets the fan with 0xC-byte draw-mode packets: texture pages
 *           0x125 then 0x25 for graphic types 1 or 2, 0x55 then 0x15 otherwise.
 *           The 636 bytes at payload offset 0x694 compare equal to the
 *           magic092/03 span at offset 0x980, which supplied the C shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfEightGouraudTrianglesFromOriginAtRadius384TintedByWorkByte9TimesEightCappedAt128BmagicMagic14403_801EF294(
    void) {
  u16 *cells;
  u16 *tint;
  POLY_G3 *primitive;
  SVECTOR *shape;
  long depth;
  long flag;
  u16 shade;
  s32 tpage;
  s32 phase;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x125;
  } else {
    tpage = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);

  cells = &D_1F800000;
  *cells = 0x180;
  PSX_REF(u16, 0x1F800024u) = (func_801782FC(0) * *(s16 *)cells) >> 12;
  PSX_REF(u16, 0x1F800026u) = (func_801783C8(0) * *(s16 *)cells) >> 12;
  shade = g_battle_work[9];
  shade = shade < 0x10 ? shade << 3 : 0x80;
  PSX_REF(u16, 0x1F80000Eu) = shade;

  i = 1;
  shape = (SVECTOR *)&D_1F800014;
  tint = &D_1F800000;
  do {
    primitive = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);

    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;

    phase = i << 9;
    shape[2].vx = (func_801782FC(phase) * *(s16 *)tint) >> 12;
    shape[2].vy = (func_801783C8(phase) * *(s16 *)tint) >> 12;
    shape[2].vz = 0;
    shape[1].vz = 0;
    shape[0].vz = 0;

    RotTransPers3(shape, shape + 1, shape + 2, (long *)&primitive->x0,
                  (long *)&primitive->x1, (long *)&primitive->x2, &depth,
                  &flag);

    i++;
    primitive->r0 = tint[7];
    primitive->g0 = tint[7];
    primitive->b0 = tint[7];
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r2 = 1;
    primitive->g2 = 1;
    primitive->b2 = 1;

    GpuAppendPrim(5, 0x1C);
  } while (i < 9);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x25;
  } else {
    tpage = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);
}
