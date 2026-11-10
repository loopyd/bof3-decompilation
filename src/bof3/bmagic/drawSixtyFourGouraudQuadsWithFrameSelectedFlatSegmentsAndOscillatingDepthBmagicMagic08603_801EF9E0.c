#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */
extern s32 D_1F800004; /* @source 0x1F800004 @kind data */
extern s32 D_1F80000C; /* @source 0x1F80000C @kind data */
extern u16 D_1F80002C; /* @source 0x1F80002C @kind data */
extern u16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern u16 D_1F800030; /* @source 0x1F800030 @kind data */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 @kind function */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC @kind function */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 @kind function */
extern s32 GetGraphType(void);

/* @source 0x801EF9E0
 * @behavior Draw sixty-four semi-transparent Gouraud quads at 64-unit angular
 *           steps. Two segment classes selected by the low three bits of the
 *           shared counter, and every segment when work byte 2 is zero, use
 *           flat geometry with radius work byte 9 times fourteen and a warm
 *           tint from that byte. Other segments use radius 192, depth offset
 *           work byte A times sixteen plus an oscillating sample, green tint,
 *           and coordinate-based packet linking. Prime the preceding edge at
 *           angle zero and carry it forward each step. Project all four
 *           vertices with RotTransPers4. Set draw modes before the band, before
 *           coordinate-linked packets, and after the band. Called without
 *           arguments by 0x801EF70C after its transform helper; returns nothing.
 * @status partial
 * @match 99.51
 * @residual Constant-one initializer scheduling and adjacent carried-edge store order differ.
 */
void drawSixtyFourGouraudQuadsWithFrameSelectedFlatSegmentsAndOscillatingDepthBmagicMagic08603_801EF9E0(void) {
  POLY_G4 *prim;
  u16 *shape;
  u16 *edge;
  s32 *radius;
  s32 *values;
  s32 *outer;
  s32 step;
  s32 phase;
  s32 nextPhase;
  s32 tpage;
  s32 selected;
  s32 selectedNext;
  s32 wobble;
  s32 x;
  s32 y;
  u16 oldX;
  u16 oldY;
  u16 newX;
  u16 newY;
  long depth;
  long flag;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);
  if (g_battle_work[2] != 0) {
    D_1F800000 = 0xC0;
  } else {
    D_1F800000 = g_battle_work[9] * 12;
  }
  radius = &D_1F800004;
  *radius = g_battle_work[9] * 14;
  D_1F80000C = g_battle_work[10] << 4;
  selected = D_80143E6C & 7;
  selectedNext = (D_80143E6C + 1) & 7;
  edge = &D_1F80002C;
  *edge = (func_801783C8(0) * D_1F800000) >> 12;
  D_1F80002E = (func_801782FC(0) * D_1F800000) >> 12;
  step = 1;
  shape = edge - 12;
  values = radius;
  outer = values - 1;
  nextPhase = 0x40;
  phase = 0;
  D_1F800030 = 0;

  for (; step < 0x41; step++) {
    if ((step & 7) == selected || (step & 7) == selectedNext || g_battle_work[2] == 0) {
      shape[0] = (func_801783C8(phase) * values[0]) >> 12;
      shape[1] = (func_801782FC(phase) * values[0]) >> 12;
      shape[2] = 0;
      shape[4] = (func_801783C8(nextPhase) * values[0]) >> 12;
      shape[5] = (func_801782FC(nextPhase) * values[0]) >> 12;
      shape[8] = shape[12];
      shape[9] = shape[13];
      shape[6] = 0;
      shape[10] = 0;
      shape[12] = (func_801783C8(nextPhase) * values[-1]) >> 12;
      shape[13] = (func_801782FC(nextPhase) * values[-1]) >> 12;
      shape[14] = 0;
    } else {
      wobble = ((step + D_80143E6C) & 0x1F) << 7;
      shape[0] = (func_801783C8(phase) * outer[0]) >> 12;
      shape[1] = (func_801782FC(phase) * outer[0]) >> 12;
      shape[2] = -(outer[3] + ((func_801782FC(wobble) << 5) >> 12));
      shape[4] = (func_801783C8(nextPhase) * outer[0]) >> 12;
      shape[5] = (func_801782FC(nextPhase) * outer[0]) >> 12;
      shape[6] = -(outer[3] + ((func_801782FC(wobble) << 5) >> 12));
      shape[10] = 0;
      shape[14] = 0;
      oldX = shape[12];
      oldY = shape[13];
      newX = shape[4];
      newY = shape[5];
      shape[12] = newX;
      shape[13] = newY;
      shape[8] = oldX;
      shape[9] = oldY;
      x = ((s16)newX << 9) + *(s32 *)(g_battle_work + 0x34);
      y = ((s16)newY << 9) + *(s32 *)(g_battle_work + 0x38);
      if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0xA5;
      } else {
        tpage = 0x35;
      }
      SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
      func_80155A08(x, y, 0, 0xC);
    }
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    RotTransPers4((SVECTOR *)shape, (SVECTOR *)(shape + 4),
                  (SVECTOR *)(shape + 8), (SVECTOR *)(shape + 12),
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &depth, &flag);
    if ((step & 7) == selected || (step & 7) == selectedNext || g_battle_work[2] == 0) {
      prim->r0 = 1;
      prim->g0 = 1;
      prim->b0 = 1;
      prim->r1 = 1;
      prim->g1 = 1;
      prim->b1 = 1;
      prim->r2 = g_battle_work[9] * 2;
      prim->g2 = g_battle_work[9] * 2;
      prim->b2 = g_battle_work[9];
      prim->r3 = g_battle_work[9] * 2;
      prim->g3 = g_battle_work[9] * 2;
      prim->b3 = g_battle_work[9];
      GpuAppendPrim(5, 0x24);
    } else {
      prim->r0 = 1;
      prim->g0 = 1;
      prim->b0 = 1;
      prim->r1 = 1;
      prim->g1 = 1;
      prim->b1 = 1;
      prim->r2 = 0x20;
      prim->g2 = 0xC0;
      prim->b2 = 0x20;
      prim->r3 = 0x20;
      prim->g3 = 0xC0;
      prim->b3 = 0x20;
      func_80155A08(x, y, 0, 0x24);
    }
    nextPhase += 0x40;
    phase += 0x40;
  }
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x25;
  } else {
    tpage = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);
}
