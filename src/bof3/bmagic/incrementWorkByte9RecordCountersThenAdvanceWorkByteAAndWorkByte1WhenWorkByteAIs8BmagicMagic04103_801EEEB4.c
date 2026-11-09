#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044. The original re-materializes the
 * cell with its own lui $vN,0x1F80 / lw $vN,0x44($vN) pair for every one of the
 * five accesses, because the counter stores below may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Base of the 12-byte counter records selected by the work object's unsigned
 * byte 0x9: the record is based at 0x80145BD4 and the three bytes this step
 * ramps are its +0x2, +0x3 and +0x4 fields, which the battle lifts write as an
 * effect factor and the sibling counter lifts on this overlay spell as the
 * selected record's counter bytes 0x0/0x1/0x2. The original folds the +1
 * displacement of the second byte into this symbol's own address
 * (lui $at,%hi(D_80145BD6 + 0x1) / lbu %lo(D_80145BD6 + 0x1)($at)), so the
 * source keeps one base plus the constant displacement rather than a distinct
 * 0x80145BD7 row, matching the area172/13 counter lift.
 * @source 0x80145BD6 @kind table
 */
extern u8 D_80145BD6[];

/* @source 0x801EEEB4
 * @behavior Overlay handler reached through the third word (index 2) of the
 *           in-image code pointer run based at 0x801EEC04, not by a call. It
 *           reads the scratchpad work-object pointer cell at 0x1F800044, takes
 *           the work object's unsigned byte at offset 0x9 as the index into the
 *           12-byte counter records at 0x80145BD6 (idx*12 spelled as
 *           sll/addu/sll through $at) and byte-adds 2 to the selected record's
 *           byte 0x0 (lbu / addiu $v0,$v0,2 / sb). It then re-reads the cell and
 *           the index byte for each of the remaining two counters and adds 1 to
 *           the selected record's byte 0x1 and byte 0x2 (lbu / addiu
 *           $v0,$v0,1 / sb), the second one addressed through the folded
 *           D_80145BD6 + 0x1 relocation. Finally it re-reads the cell once more,
 *           increments the work object's byte at offset 0xA (lbu / addiu 1 /
 *           sb), zero-extends that stored byte with andi 0xFF and compares it
 *           with 8 (addiu $v0,$zero,8 / bne); only on equality does it re-read
 *           the cell with its own lui/lw pair and increment the work object's
 *           state byte at offset 0x1, which steps the overlay work byte 1
 *           dispatcher at 0x801EF190. Leaf with no frame, no saved registers and
 *           no calls; takes no arguments, returns nothing and reads or writes no
 *           other state.
 *
 *           Shape: the counter ramp is the shape of the exact counter-triplet
 *           lifts on 0x80145BD6 (incrementWorkByteBRecordCountersAndWorkByte1-
 *           WhenWorkByte9Is10BmagicMagic01000_801EED0C, which keeps the
 *           incremented byte in the stored-then-masked form this function uses
 *           for the 0xA compare, and the area172/13 lift that folds +1/+2 into
 *           the one base); this function's own index byte 0x9, its 2/1/1 step
 *           values and its compared literal 8 replace theirs. The 272 bytes
 *           0x801EEEB4..0x801EEFC3 match no byte-identical span in any other
 *           extracted EMI image, so no clone applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9RecordCountersThenAdvanceWorkByteAAndWorkByte1WhenWorkByteAIs8BmagicMagic04103_801EEEB4(void) {
  u32 offset;

  offset = g_battle_work[9] * 12;
  D_80145BD6[offset] += 2;
  offset = g_battle_work[9] * 12;
  D_80145BD6[offset + 1] += 1;
  offset = g_battle_work[9] * 12;
  D_80145BD6[offset + 2] += 1;
  if (++g_battle_work[0xA] == 8) {
    g_battle_work[1]++;
  }
}
