#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 distance-gated per-frame chance: scales byte 3 of the
 * D_801E320C record into the span ((byte * 8) + 0x18) * (arg1 & 0xFF), reduces
 * it to its low byte, measures the absolute distance between the signed byte
 * 0xA of the scratch work record at 0x1F800044 and the low byte of arg0, and
 * returns whether a 4-bit rand() draw falls below the span
 * quarter/half/full/quadruple ladder 1, 2, 4, 8, 0xE. The first three ladder
 * comparisons are unsigned (original sltu) and the fourth is signed (original
 * slt), so the distance is unsigned and the quadruple test is cast to s32.
 * @source 0x801E02D4
 * @status partial
 * @match 81.25
 * @residual entry-block allocation/scheduling: the original keeps the
 *   ((byte * 8) + 0x18) intermediate in $v0, reuses $v0 for the scratch
 *   pointer, keeps the multiply result in $LO (mflo a2 lands after the abs
 *   negu) and re-masks the selected level before the rand() compare; this
 *   clean-C shape materializes the product/limit in $a2 and the scratch
 *   pointer in $v1 and drops the level's byte mask. Same 48-instruction /
 *   192-byte size, first live difference at instruction 8
 *   (addiu a2,v0,0x18 vs addiu v0,v0,0x18); byte-match DIFFER.
 */
s32 func_801E02D4(u32 arg0, u32 arg1) {
  u8* work;
  u32 span;
  u32 limit;
  s32 diff;
  u32 distance;
  s32 level;

  span = (((u8*)D_801E320C)[3] * 8 + 0x18);
  span = span * (arg1 & 0xFF);
  work = D_1F800044;
  diff = (s8)work[0xA] - (s32)(arg0 & 0xFF);
  if (diff < 0) {
    diff = -diff;
  }
  distance = (u32)(diff & 0xFF);
  limit = span & 0xFF;

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
  return (rand() & 0xF) < (level & 0xFF);
}
