#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s32 D_80149308; /* @source 0x80149308 @kind data */
extern s32 D_8014930C; /* @source 0x8014930C @kind data */
extern SVECTOR D_1F800014[]; /* @source 0x1F800014 @kind data */
extern void func_801DD350(s32 work); /* @source 0x801DD350 */
extern void func_80155A08(s32 x, s32 y, s32 mode, s32 size); /* @source 0x80155A08 */
extern s32 GetGraphType(void);

/* @source 0x801EFA80
 * @behavior Draw a semi-transparent Gouraud quad from work-byte-9-dependent
 *           offsets transformed through helper 0x801DD350. Retain the first
 *           offset plus shared words 0x80149308 and 0x8014930C as the packet
 *           submission coordinates. Build four scratchpad vectors from work
 *           words 34 and 38 and the transformed offsets, with depths from
 *           work halfword 3E and word 14. Project with RotAverage4, tint the
 *           vertices (16,1,1), (16,1,1), (224,64,64) and (1,1,1), and submit
 *           through helper 0x80155A08. Select draw page 0x2A5 for graph types
 *           1 and 2, otherwise 0xB5. Takes no arguments and returns no value.
 * @status partial
 * @match 83.41
 * @residual 181/217 instructions; 856 original bytes versus 868 current.
 *           Scratchpad base reuse and later scheduling remain different.
 */
void drawTranslucentGouraudQuadFromWorkByte9OffsetsBmagicMagic11403_801EFA80(void) {
  u8 *work0;
  u8 *work1;
  u8 *work2;
  u8 *work3;
  u8 *work4;
  u8 *work5;
  s32 x;
  s32 y;
  s32 page;
  POLY_G4 *prim;
  SVECTOR *shape;
  long p;
  long flag;

  work0 = g_battle_work;
  *(s32 *)(work0 + 0xC) = 0x28000;
  *(s32 *)(work0 + 0x10) = 0xA0000 - (work0[9] << 17);
  func_801DD350((s32)work0);
  work1 = g_battle_work;
  x = D_80149308 + *(s32 *)(work1 + 0xC);
  y = D_8014930C + *(s32 *)(work1 + 0x10);
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    page = 0x2A5;
  } else {
    page = 0xB5;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, page, 0);
  func_80155A08(x, y, 0, 0xC);
  prim = (POLY_G4 *)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  work2 = g_battle_work;
  *(s32 *)(work2 + 0xC) = 0;
  *(s32 *)(work2 + 0x10) = -0x1000;
  func_801DD350((s32)work2);
  work3 = g_battle_work;
  shape = D_1F800014;
  shape[0].vx = ((*(s32 *)(work3 + 0x34) + *(s32 *)(work3 + 0xC)) >> 9) - 0x4000;
  SPAD_REF(s16, 0x16u) = ((*(s32 *)(work3 + 0x38) + *(s32 *)(work3 + 0x10)) >> 9) - 0x4000;
  SPAD_REF(s16, 0x18u) = -*(s16 *)(work3 + 0x3E) / 2;
  *(s32 *)(work3 + 0xC) = 0;
  *(s32 *)(work3 + 0x10) = 0x1000;
  func_801DD350((s32)work3);
  work4 = g_battle_work;
  shape[1].vx = ((*(s32 *)(work4 + 0x34) + *(s32 *)(work4 + 0xC)) >> 9) - 0x4000;
  SPAD_REF(s16, 0x1Eu) = ((*(s32 *)(work4 + 0x38) + *(s32 *)(work4 + 0x10)) >> 9) - 0x4000;
  SPAD_REF(s16, 0x20u) = -*(s16 *)(work4 + 0x3E) / 2;
  *(s32 *)(work4 + 0xC) = 0x28000;
  *(s32 *)(work4 + 0x10) = 0xA8000 - work4[9] * 0x18000;
  func_801DD350((s32)work4);
  work5 = g_battle_work;
  shape[3].vx = ((D_80149308 + *(s32 *)(work5 + 0xC)) >> 9) - 0x4000;
  SPAD_REF(s16, 0x2Eu) = ((D_8014930C + *(s32 *)(work5 + 0x10)) >> 9) - 0x4000;
  shape[2].vx = (x >> 9) - 0x4000;
  SPAD_REF(s16, 0x26u) = (y >> 9) - 0x4000;
  SPAD_REF(s16, 0x30u) = -*(s32 *)(work5 + 0x14) / 2;
  SPAD_REF(s16, 0x28u) = -*(s32 *)(work5 + 0x14) / 2;
  RotAverage4(shape, shape + 1, shape + 2, shape + 3,
              (long *)&prim->x0, (long *)&prim->x1,
              (long *)&prim->x2, (long *)&prim->x3, &p, &flag);
  prim->r0 = 0x10;
  prim->r1 = 0x10;
  prim->r2 = 0xE0;
  prim->g0 = 1;
  prim->b0 = 1;
  prim->g1 = 1;
  prim->b1 = 1;
  prim->g2 = 0x40;
  prim->b2 = 0x40;
  prim->r3 = 1;
  prim->g3 = 1;
  prim->b3 = 1;
  func_80155A08(x, y, 0, 0x24);
}
