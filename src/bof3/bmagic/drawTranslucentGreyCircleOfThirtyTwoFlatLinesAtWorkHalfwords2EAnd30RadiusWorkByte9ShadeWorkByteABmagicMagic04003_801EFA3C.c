#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern void func_8017AA80(void *primitive); /* @source 0x8017AA80 @kind function */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */
extern s32 GetGraphType(void);

/* @source 0x801EFA3C
 * @behavior Draws a semi-transparent grey circle of 32 flat line primitives,
 * centred at work halfwords 0x2E and 0x30, with radius from work byte 9 and
 * equal RGB channels from work byte 0xA. Called by 0x801EF248. Takes no
 * arguments and returns nothing. Publishes the centre, radius and shade into
 * scratchpad halfwords 0, 2, 4 and 0xA. Each line joins phase i*0x80 to
 * (i+1)*0x80 using the signed samples returned by 0x801782FC and 0x801783C8,
 * multiplied by the signed radius and shifted right 12. Publishes each phase
 * at scratchpad halfword 8 and reloads it for the second sample. Wraps the
 * lines in draw-mode packets selecting 0xA5 then 0x25 for graph types 1 or 2,
 * or 0x35 then 0x15 otherwise. Queues mode packets with (3, 0xC) and lines
 * with (3, 0x10).
 * @status partial
 * @match 97.86
 * @residual 137/140 instructions at the original 560-byte size. The raw
 * phase shift is scheduled before the sign-extended argument at both sample
 * sites; the second phase store uses a1 instead of v0.
 */
void drawTranslucentGreyCircleOfThirtyTwoFlatLinesAtWorkHalfwords2EAnd30RadiusWorkByte9ShadeWorkByteABmagicMagic04003_801EFA3C(void) {
  u8 *work;
  u8 *primitive;
  u16 *cells;
  s16 *phase;
  u16 *shades;
  s32 i;
  s32 graphValue;
  s32 step;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graphValue = 0xA5;
  } else {
    graphValue = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graphValue, 0);
  GpuAppendPrim(3, 0xC);
  work = SPAD_PTR_SLOT(u8, 0x44u);
  cells = &D_1F800000;
  *cells = FIELD_REF(u16, work, 0x2E);
  SPAD_REF(u16, 0x02u) = FIELD_REF(u16, work, 0x30);
  SPAD_REF(u16, 0x0Au) = work[0xA];
  SPAD_REF(u16, 0x04u) = work[9];
  i = 0;
  phase = (s16 *)(cells + 4);
  shades = cells;
  for (; i < 0x20; i++) {
    primitive = g_PrimCursor;
    func_8017AA80(primitive);
    SetSemiTrans(primitive, 1);
    step = func_801782FC(*phase = i << 7);
    FIELD_REF(u16, primitive, 8) = ((u16 *)phase)[-4] + ((step * phase[-2]) >> 12);
    step = func_801783C8(*phase);
    FIELD_REF(u16, primitive, 0xA) = ((u16 *)phase)[-3] + ((step * phase[-2]) >> 12);
    step = func_801782FC(*phase = (i + 1) << 7);
    FIELD_REF(u16, primitive, 0xC) = ((u16 *)phase)[-4] + ((step * phase[-2]) >> 12);
    step = func_801783C8(*phase);
    FIELD_REF(u16, primitive, 0xE) = ((u16 *)phase)[-3] + ((step * phase[-2]) >> 12);
    primitive[4] = shades[5];
    primitive[5] = shades[5];
    primitive[6] = shades[5];
    GpuAppendPrim(3, 0x10);
  }
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graphValue = 0x25;
  } else {
    graphValue = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graphValue, 0);
  GpuAppendPrim(3, 0xC);
}
