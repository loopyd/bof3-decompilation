#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E02D4
 * @behavior AREA030 distance-gated per-frame chance: scales byte 3 of the
 * D_801E320C record into the span ((byte * 8) + 0x18) * (arg1 & 0xFF), reduces
 * it to its low byte, measures the absolute distance between the signed byte
 * 0xA of the scratch work record at 0x1F800044 and the low byte of arg0, and
 * returns whether a 4-bit rand() draw falls below the span
 * quarter/half/full/quadruple ladder 1, 2, 4, 8, 0xE. The first three ladder
 * comparisons are unsigned (original sltu) and the fourth is signed (original
 * slt), so the distance is unsigned and the quadruple test is cast to s32.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Two shape details reproduce the original code exactly. The span product stays in
 * its own u32 (`prod`) so the multiply is the entry block's mult/mflo pair, and the
 * selected level is narrowed into a separate signed variable (`chance`): masking the
 * level inline, either in the ladder arm or in the return expression, lets the
 * canonical -O2 build fold the zero extension, while a distinct destination keeps
 * the `andi s0,s0,0xff` in the draw call's delay slot (192/192 bytes, 48/48
 * instructions).
 */
s32 func_801E02D4(u32 arg0, u32 arg1) {
  u8* work;
  u32 span;
  u32 prod;
  u32 limit;
  s32 diff;
  u32 distance;
  s32 level;
  s32 chance;

  span = (((u8*)D_801E320C)[3] * 8 + 0x18);
  prod = span * (arg1 & 0xFF);
  work = D_1F800044;
  diff = (s8)work[0xA] - (s32)(arg0 & 0xFF);
  distance = (u32)(diff < 0 ? -diff : diff) & 0xFF;
  limit = prod & 0xFF;
  if (distance < (limit >> 2)) {
    level = 1;
  } else if (distance < (limit >> 1)) {
    level = 2;
  } else if (distance < limit) {
    level = 4;
  } else if ((s32)distance < (s32)(limit * 4)) {
    level = 8;
  } else {
    level = 0xE;
  }
  chance = level & 0xFF;
  return (rand() & 0xF) < chance;
}
