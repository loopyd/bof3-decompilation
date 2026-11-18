#include "bof3/world/area03004_internal.h"

/* Engine primitive initialiser of the main executable: it writes the tag and the
 * code byte of the 0x10-byte packet that g_PrimCursor points at. This payload
 * only calls it, and the address lies outside every EMI payload.
 * @source 0x8017AA80 @kind function */
extern void func_8017AA80(void* primitive);

/* AREA030 per-slot panel records based at 0x80146888, stride 0x98 (152 bytes).
 * The original reaches four of their byte and word fields and one signed
 * halfword through their own %hi/%lo symbol pair, each access re-reading the
 * slot index byte D_801E3208, so the fields stay separate address-anchored
 * objects rather than one typed struct view. */
extern u8  D_80146889[];   /* @source 0x80146889 @kind table */
extern u8  D_8014688C[];   /* @source 0x8014688C @kind table */
extern u8  D_80146892[];   /* @source 0x80146892 @kind table */
extern u8  D_80146898[];   /* @source 0x80146898 @kind table */
extern u8  D_80146916[];   /* @source 0x80146916 @kind table */

/* @source 0x801D2DC4
 * @behavior entry 2 of the local handler table at 0x801E1D80, which the
 * dispatcher func_801D2BF0 indexes with the work record's unsigned step byte at
 * +0x1: it first selects draw mode (0, 2) and submits panel sprite 0x11 at
 * (0x87, 0xB4), then advances the work record's halfword at +0x30 towards the
 * signed byte that the slot index D_801E3208 selects from D_80146892 (stride
 * 0x98). A zero direction word at +0x10 arms the approach: while the halfword
 * is farther from that byte than 4 * D_801E320C[3] + 0xC the record's byte at
 * +0x9 counts up to 0xF and the direction word is then latched to -1 or 1 by
 * the sign of the gap, while a halfword already inside that band clears byte
 * +0x9. Every pass adds the direction word to the halfword, clamps the sum into
 * [4 * D_801E320C[3] - 0x24, 0x24 - 4 * D_801E320C[3]] and stores it back.
 * It then emits a grey 0x10-byte line at x 0xB9 from y 0xC9 to y 0xD5, draws
 * the panel row through func_801D3244 (origin halfword 0xAD below the row, 0xCB,
 * (8 * D_801E320C[3] + 0x18) & 0xF8, work byte 0xA low three bits doubled, 2,
 * and a flag that is 1 when the row start falls outside the 8 * D_801E320C[3] +
 * 0x18 span above it), and draws the panel cell through
 * drawMirroredPanelCellQuadWorld00Area03004_801D2C34 (the D_80146892 byte plus
 * 0xB9 clamped to 0x90..0xE4, 0xCB, whether the D_80146898 word's top bit is
 * clear, 2). When D_80146889's byte is 4 it first emits a second 0x10-byte line
 * in the 0x80 red at y 0xD3 whose length is the signed halfword at D_80146916
 * scaled by 96/256.
 * Finally, when D_80145E92 is 4, D_8014688C's byte is not 3, the clamped cell
 * origin falls outside the span above it and the frame counter D_80143E6C low
 * nibble is clear, it requests sound cue 0x207 through func_8015DF18.
 * @status exact
 * @match 100.00
 * @residual none
 */
void slideWorkHalfword30TowardSlotTargetAndDrawPanelWorld00Area03004_801D2DC4(void) {
  u8*      approachWork;
  u8*      work;
  u8*      rowWork;
  u8*      sumWork;
  u8*      clampMode;
  LINE_F2* prim;
  s32      clampScale;
  s32      rowScale;
  s32      cur;
  s32      rowPosition;
  s32      target;
  s32      sum;
  u16      limitedPosition;
  s32      defaultPosition;
  s32      lowerBound;
  s32      belowLower;
  s32      upperBound;
  s32      rowUpper;
  s32      rowWidth;
  u16      upperSnapshot;
  u16      lowerSnapshot;
  s8       level;
  s16      clamped;
  s16      rowCell;
  s32      comparedCell;
  u8*      rowMode;
  u32      rowIndex;

  submitTpageDrawMode(0, 2);
  func_801E0DCC(0x11, 2, 0x87, 0xB4);

  approachWork = D_1F800044;
  if (*(s32*)(approachWork + 0x10) == 0) {
    s32 approachTarget;
    s32 approachBand;
    s32 approachValue;
    s32 approachMode;
    u32 approachIndex;

    do {
      approachIndex = D_801E3208 * 19;
      cur = (s32)*(s16*)(approachWork + 0x30);
    } while (0);
    approachMode = ((u8*)D_801E320C)[3];
    approachTarget = (s8)D_80146892[approachIndex * 8];
    approachBand = approachMode * 4;
    approachValue = cur + approachBand;
    approachValue += 0xC;
    if (approachValue < approachTarget || approachTarget < cur - approachBand - 0xC) {
      if (approachWork[9] >= 0xF) {
        s8 directionTarget = (s8)approachTarget;
        if (directionTarget < cur) {
          *(s32*)(approachWork + 0x10) = -1;
        } else if (cur < directionTarget) {
          *(s32*)(approachWork + 0x10) = 1;
        }
      } else {
        approachWork[9] = approachWork[9] + 1;
      }
    } else {
      approachWork[9] = 0;
    }
  } else {
    approachWork[9] = 0;
  }

  sumWork = D_1F800044;
  lowerBound = *(s32*)(sumWork + 0x10);
  sum = (s32)*(u16*)(sumWork + 0x30) + lowerBound;
  *(u16*)(sumWork + 0x30) = (u16)sum;
  work = sumWork;
  do {
    sum = (s32)((u32)sum << 16);
    clampMode = (u8*)D_801E320C;
    sum >>= 16;
  } while (0);
  clampScale = clampMode[3];
  defaultPosition = *(u16*)(work + 0x30);
  do {
    lowerBound = clampScale * 4 - 0x24;
    belowLower = sum < lowerBound;
  } while (0);
  limitedPosition = (u16)lowerBound;
  if (!belowLower) {
    upperBound = 0x24 - clampScale * 4;
    limitedPosition = (u16)upperBound;
    if (sum <= upperBound) {
      limitedPosition = defaultPosition;
    }
  }
  prim = (LINE_F2*)g_PrimCursor;
  *(u16*)(work + 0x30) = (u16)limitedPosition;
  func_8017AA80(prim);
  prim->x0 = 0xB9;
  prim->x1 = 0xB9;
  prim->y0 = 0xC9;
  prim->y1 = 0xD5;
  prim->r0 = 0x80;
  prim->g0 = 0x80;
  prim->b0 = 0x80;
  func_8014E5A0(2, 0x10);

  do {
    do {
      rowMode = (u8*)D_801E320C;
      rowIndex = D_801E3208;
    } while (0);
    rowWork = D_1F800044;
    rowScale = rowMode[3];
    level = D_80146892[rowIndex * 0x98];
  } while (0);
  do {
    rowPosition = *(u16*)(rowWork + 0x30);
  } while (0);
  rowWidth = rowScale * 8;
  cur = rowWidth + 0x18;
  rowCell = (s16)level;
  do {
    rowScale *= 4;
    rowPosition -= rowScale;
    rowScale = rowPosition + 0xAD;
  } while (0);
  rowUpper = rowScale + rowWidth + 0x18;
  upperSnapshot = (u16)rowUpper;
  clamped = rowCell + 0xB9;
  lowerSnapshot = (u16)rowScale;
  do {
    comparedCell = (s16)clamped;
  } while (0);
  if (((s16)rowUpper < comparedCell) || (comparedCell < (s16)rowScale)) {
    func_801D3244((s16)rowScale, 0xCB, cur & 0xF8, (rowWork[0xA] & 7) * 2, 2, 1);
  } else {
    func_801D3244((s16)rowScale, 0xCB, cur & 0xF8, (rowWork[0xA] & 7) * 2, 2, 0);
  }

  if (D_80146889[D_801E3208 * 0x98] == 4) {
    prim = (LINE_F2*)g_PrimCursor;
    func_8017AA80(prim);
    prim->x0 = 0x88;
    prim->x1 = 0x88 + (*(s16*)(D_80146916 + D_801E3208 * 0x98) * 96) / 256;
    prim->y0 = 0xD3;
    prim->y1 = 0xD3;
    prim->r0 = 0x80;
    prim->g0 = 0;
    prim->b0 = 0;
    func_8014E5A0(2, 0x10);

    target = (s8)(~(*(u32*)(D_80146898 + D_801E3208 * 0x98)) >> 31);
    clamped = (s16)clamped < 0x90
                  ? 0x90
                  : ((s16)clamped < 0xE5 ? clamped : 0xE4);
    drawMirroredPanelCellQuadWorld00Area03004_801D2C34(clamped, 0xCB, (s8)target, 2);

    if ((D_80145E92 == 4) && (D_8014688C[D_801E3208 * 0x98] != 3) &&
        (((s16)upperSnapshot < clamped) || (clamped < (s16)lowerSnapshot)) &&
        ((D_80143E6C & 0xF) == 0)) {
      func_8015DF18(0x207);
    }
  }
}
