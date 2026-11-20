#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E02D4
 * @behavior Scale record byte 3 by 8, add 0x18 and multiply by the low byte of
 * spanScale. Take the low byte of that product and of abs((s8)work[0xA] -
 * (s32)(referenceByte & 0xFF)). The original unsigned and signed comparisons
 * select thresholds 1, 2, 4, 8 or 14; return whether rand() & 15 is below the
 * selected threshold.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Two shape details reproduce the original code exactly. The span product stays in
 * its own u32 (`spanProduct`) so the multiply is the entry block's mult/mflo pair, and the
 * selected chanceLevel is narrowed into a separate signed variable (`chanceThreshold`): masking the
 * chanceLevel inline, either in the ladder arm or in the return expression, lets the
 * canonical -O2 build fold the zero extension, while a distinct destination keeps
 * the `andi s0,s0,0xff` in the draw call's delay slot (192/192 bytes, 48/48
 * instructions).
 */
s32 testWorkByteDistanceChanceWorld00Area03004(u32 referenceByte, u32 spanScale) {
  u8* work;
  u32 span;
  u32 spanProduct;
  u32 byteLimit;
  s32 signedDifference;
  u32 byteDistance;
  s32 chanceLevel;
  s32 chanceThreshold;

  span = (((u8*)D_801E320C)[3] * 8 + 0x18);
  spanProduct = span * (spanScale & 0xFF);
  work = D_1F800044;
  signedDifference = (s8)work[0xA] - (s32)(referenceByte & 0xFF);
  byteDistance = (u32)(signedDifference < 0 ? -signedDifference : signedDifference) & 0xFF;
  byteLimit = spanProduct & 0xFF;
  if (byteDistance < (byteLimit >> 2)) {
    chanceLevel = 1;
  } else if (byteDistance < (byteLimit >> 1)) {
    chanceLevel = 2;
  } else if (byteDistance < byteLimit) {
    chanceLevel = 4;
  } else if ((s32)byteDistance < (s32)(byteLimit * 4)) {
    chanceLevel = 8;
  } else {
    chanceLevel = 0xE;
  }
  chanceThreshold = chanceLevel & 0xFF;
  return (rand() & 0xF) < chanceThreshold;
}
