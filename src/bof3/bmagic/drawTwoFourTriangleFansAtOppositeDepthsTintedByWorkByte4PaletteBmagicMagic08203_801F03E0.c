#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern s16 D_1F80000E; /* @source 0x1F80000E @kind data */
extern s16 D_1F800014; /* @source 0x1F800014 @kind data */
extern u8 D_801F0C9C[]; /* @source 0x801F0C9C @kind data */
extern u8 D_801F0C9D[]; /* @source 0x801F0C9D @kind data */
extern u8 D_801F0C9E[]; /* @source 0x801F0C9E @kind data */
extern u8 D_801F0CCC[]; /* @source 0x801F0CCC @kind data */
extern u8 D_801F0CCD[]; /* @source 0x801F0CCD @kind data */
extern u8 D_801F0CCE[]; /* @source 0x801F0CCE @kind data */
extern s32 GetGraphType(void);
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */
extern void func_800AF0F8(u32 x, u32 y, s32 *keys, u8 *packets,
                         u8 count, u8 stride, u8 extra); /* @source 0x800AF0F8 */

/* @source 0x801F03E0
 * @behavior Draw two four-triangle Gouraud fans with radius 48, centred at
 *           depths 80 and -80. Step the phase by eight modulo 32 from work
 *           byte 9. Select each triangle's RGB from the work-byte-4 palette
 *           row, using tables at 0x801F0C9C and 0x801F0CCC respectively.
 *           Project each fan with RotAverage3, retain its four depth keys and
 *           submit its four 28-byte packets through helper 0x800AF0F8 at
 *           work words 0x34 and 0x38. Seed one draw-mode packet with tpage
 *           0x25 for graph types 1 and 2, otherwise 0x15. Takes no arguments;
 *           callers at 0x801EFEF4, 0x801EFFE8, 0x801F00DC and 0x801F0260
 *           consume no result. Shared helper roles remain address-anchored.
 * @status partial
 * @match 59.88
 * @residual 288/425 instructions; 1924/1700 bytes. Scalar scratchpad
 *           offsets rematerialize addresses; frame is 136 rather than 144.
 *           Clean-C representation and scheduling remain unresolved.
 */
void drawTwoFourTriangleFansAtOppositeDepthsTintedByWorkByte4PaletteBmagicMagic08203_801F03E0(void) {
  s32 keys[16];
  long depth;
  long flag;
  POLY_G3 *primitive;
  u8 *packets;
  s16 *scratch;
  s16 *radius;
  SVECTOR *shape;
  s32 phase;
  s32 next_phase;
  s32 tpage;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x25;
  } else {
    tpage = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                FIELD_REF(s32, g_battle_work, 0x38u), 2, 0xC);
  packets = g_PrimCursor;
  for (phase = g_battle_work[9]; phase < g_battle_work[9] + 32;
       phase = next_phase) {
    scratch = &D_1F800002;
    radius = scratch - 1;
    shape = (SVECTOR *)&D_1F800014;
    scratch[0] = 80;
    scratch[-1] = 48;
    scratch[6] = (phase - g_battle_work[9]) / 8;
    scratch[3] = D_801F0C9C[g_battle_work[4] * 12 + scratch[6] * 3];
    scratch[4] = D_801F0C9D[g_battle_work[4] * 12 + scratch[6] * 3];
    scratch[5] = D_801F0C9E[g_battle_work[4] * 12 + scratch[6] * 3];
    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[0].vz = 80;
    scratch[1] = (phase & 31) << 7;
    shape[1].vx = (func_801782FC(scratch[1]) * *radius) >> 12;
    shape[1].vy = (func_801783C8(scratch[1]) * *radius) >> 12;
    next_phase = phase + 8;
    shape[1].vz = 0;
    scratch[1] = (next_phase & 31) << 7;
    shape[2].vx = (func_801782FC(scratch[1]) * *radius) >> 12;
    shape[2].vy = (func_801783C8(scratch[1]) * *radius) >> 12;
    primitive = (POLY_G3 *)g_PrimCursor;
    shape[2].vz = 0;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 0);
    keys[(phase - g_battle_work[9]) / 8] =
        RotAverage3(shape, shape + 1, shape + 2, (long *)&primitive->x0,
                    (long *)&primitive->x1, (long *)&primitive->x2, &depth, &flag);
    primitive->r0 = (u16)scratch[3];
    primitive->g0 = (u16)scratch[4];
    primitive->b0 = (u16)scratch[5];
    primitive->r1 = (u16)scratch[3];
    primitive->g1 = (u16)scratch[4];
    primitive->b1 = (u16)scratch[5];
    primitive->r2 = (u16)scratch[3];
    primitive->g2 = (u16)scratch[4];
    primitive->b2 = (u16)scratch[5];
    g_PrimCursor += 0x1C;
  }
  func_800AF0F8(FIELD_REF(u32, g_battle_work, 0x34u),
                FIELD_REF(u32, g_battle_work, 0x38u), keys, packets, 4, 0x1C, 2);
  packets = g_PrimCursor;
  for (phase = g_battle_work[9]; phase < g_battle_work[9] + 32;
       phase = next_phase) {
    scratch = &D_1F80000E;
    shape = (SVECTOR *)&D_1F800014;
    scratch[0] = (phase - g_battle_work[9]) / 8;
    scratch[-3] = D_801F0CCC[g_battle_work[4] * 12 + scratch[0] * 3];
    scratch[-2] = D_801F0CCD[g_battle_work[4] * 12 + scratch[0] * 3];
    scratch[-1] = D_801F0CCE[g_battle_work[4] * 12 + scratch[0] * 3];
    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[0].vz = -(u16)scratch[-6];
    scratch[-5] = (phase & 31) << 7;
    shape[1].vx = (func_801782FC(scratch[-5]) * scratch[-7]) >> 12;
    shape[1].vy = (func_801783C8(scratch[-5]) * scratch[-7]) >> 12;
    next_phase = phase + 8;
    shape[1].vz = 0;
    scratch[-5] = (next_phase & 31) << 7;
    shape[2].vx = (func_801782FC(scratch[-5]) * scratch[-7]) >> 12;
    shape[2].vy = (func_801783C8(scratch[-5]) * scratch[-7]) >> 12;
    primitive = (POLY_G3 *)g_PrimCursor;
    shape[2].vz = 0;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 0);
    keys[(phase - g_battle_work[9]) / 8] =
        RotAverage3(shape, shape + 1, shape + 2, (long *)&primitive->x0,
                    (long *)&primitive->x1, (long *)&primitive->x2, &depth, &flag);
    primitive->r0 = (u16)scratch[-3];
    primitive->g0 = (u16)scratch[-2];
    primitive->b0 = (u16)scratch[-1];
    primitive->r1 = (u16)scratch[-3];
    primitive->g1 = (u16)scratch[-2];
    primitive->b1 = (u16)scratch[-1];
    primitive->r2 = (u16)scratch[-3];
    primitive->g2 = (u16)scratch[-2];
    primitive->b2 = (u16)scratch[-1];
    g_PrimCursor += 0x1C;
  }
  func_800AF0F8(FIELD_REF(u32, g_battle_work, 0x34u),
                FIELD_REF(u32, g_battle_work, 0x38u), keys, packets, 4, 0x1C, 2);
}
