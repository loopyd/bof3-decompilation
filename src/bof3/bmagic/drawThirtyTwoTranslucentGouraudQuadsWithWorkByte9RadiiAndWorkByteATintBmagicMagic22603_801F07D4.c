#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern s16 D_1F800014; /* @source 0x1F800014 @kind data */
extern s32 func_801782FC(s32 phase);
extern s32 func_801783C8(s32 phase);
extern s32 GetGraphType(void);
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */

/* @source 0x801F07D4
 * @behavior Draws thirty-two translucent Gouraud quads under the transform
 *           installed by the caller at 0x801F053C. Work byte 9 seeds radii
 *           scaled by 24 and 32; work byte A seeds RGB halfwords scaled by
 *           12, 6 and 6. Primes both edges at phase zero, with the larger
 *           radius at depth -64 and the smaller at depth zero. Each pass
 *           carries the previous edge and samples the next at step times
 *           128. Work words 0x34 and 0x38 plus the smaller-radius sample
 *           shifted left nine select the output location. Emits a draw-mode
 *           packet using tpage 0xA5 for graph types 1 or 2, otherwise 0x35,
 *           then projects a semi-transparent POLY_G4. The first two vertices
 *           take RGB 1; the last two take independent tint-halfword reads.
 *           Links packets of sizes 0xC and 0x28 through 0x80155A08. Takes
 *           no arguments and returns nothing; other work-field roles remain
 *           unknown.
 * @status partial
 * @match 96.64
 * @residual Initial work-pointer register and loop constant scheduling differ.
 */
void drawThirtyTwoTranslucentGouraudQuadsWithWorkByte9RadiiAndWorkByteATintBmagicMagic22603_801F07D4(void) {
  u8 *work;
  s16 *radius;
  s16 *next;
  s16 *mid;
  s16 *outer;
  s16 *tint;
  SVECTOR *shape;
  POLY_G4 *prim;
  s32 phase;
  s32 step;
  s32 x;
  s32 y;
  s32 tpage;
  long depth;
  long flag;

  work = g_battle_work;
  radius = &D_1F800000;
  *radius = work[9] * 24;
  D_1F800002 = work[9] << 5;
  SPAD_REF(u16, 0x0Au) = work[0xA] * 12;
  SPAD_REF(u16, 0x0Cu) = work[0xA] * 6;
  SPAD_REF(u16, 0x0Eu) = work[0xA] * 6;

  mid = &D_1F800014;
  *mid = (func_801782FC(0) * D_1F800002) >> 12;
  SPAD_REF(s16, 0x16u) = (func_801783C8(0) * D_1F800002) >> 12;
  SPAD_REF(s16, 0x20u) = -64;
  SPAD_REF(s16, 0x18u) = -64;
  SPAD_REF(s16, 0x24u) = (func_801782FC(0) * *radius) >> 12;
  SPAD_REF(s16, 0x26u) = (func_801783C8(0) * *radius) >> 12;

  step = 1;
  next = mid + 4;
  outer = radius + 1;
  shape = (SVECTOR *)mid;
  tint = radius + 5;
  SPAD_REF(s16, 0x30u) = 0;
  SPAD_REF(s16, 0x28u) = 0;

  for (; step < 0x21; step++) {
    phase = step << 7;
    next[0] = (u16)next[-4];
    next[1] = (u16)next[-3];
    next[-4] = (func_801782FC(phase) * outer[0]) >> 12;
    next[-3] = (func_801783C8(phase) * outer[0]) >> 12;
    next[8] = (u16)next[4];
    next[9] = (u16)next[5];
    next[4] = (func_801782FC(phase) * outer[-1]) >> 12;
    next[5] = (func_801783C8(phase) * outer[-1]) >> 12;
    work = g_battle_work;
    x = (next[4] << 9) + *(s32 *)(work + 0x34);
    y = (next[5] << 9) + *(s32 *)(work + 0x38);
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      tpage = 0xA5;
    } else {
      tpage = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    func_80155A08(x, y, 0, 0xC);
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    RotTransPers4(shape, shape + 1, shape + 2, shape + 3,
                 (long *)&prim->x0, (long *)&prim->x1,
                 (long *)&prim->x2, (long *)&prim->x3, &depth, &flag);
    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = (u16)tint[0];
    prim->g2 = (u16)tint[1];
    prim->b2 = (u16)tint[2];
    prim->r3 = (u16)tint[0];
    prim->g3 = (u16)tint[1];
    prim->b3 = (u16)tint[2];
    func_80155A08(x, y, 0, 0x28);
  }
}
