#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-materializes the cell address (lui $v0,0x1F80 / lw $v0,0x44)
 * once per counter group and once for each of the countdown tail's two groups,
 * so the cell is reached through this target-map symbol rather than through a
 * constant-address accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* The three counter bytes this handler advances, at the start of the 12-byte
 * records selected by work byte 0xA. The original reaches the second and third
 * counters through their own addresses, so each byte keeps its own array - the
 * form the exact sibling storeDoubleWorkByte9IntoWorkByteARecordTripletAnd
 * AdvanceWorkByte1WhenAtLeast6BmagicMagic11503_801EEDDC uses for this table.
 * The bytes at 0x80145BD6 and 0x80145BD8 also arrive from this target's
 * generated undefined_syms_auto.txt, and 0x80145BD7 binds from its
 * address-encoded name.
 * @source 0x80145BD6 @kind table
 */
extern u8 D_80145BD6[];
extern u8 D_80145BD7[];
extern u8 D_80145BD8[];

/* @source 0x801EF23C
 * @behavior Per-frame handler of this overlay's handler run: it reads the
 * scratchpad work-object pointer cell at 0x1F800044 once per counter group and,
 * for each of the record's three counter bytes, re-reads the work object's
 * unsigned byte 0xA, scales it by the 12-byte record stride into a byte offset
 * and increments the selected 0x80145BD6 record's bytes 0x0/0x1/0x2 with a
 * byte-width read-modify-write (lbu, addiu $v0,$v0,1, sb). It then re-reads the
 * cell, and the work object's unsigned byte 9 with it, and decrements that byte
 * with the same byte-width form before testing the stored value against zero
 * (andi $v0,$v0,0xFF, bnez). Only when the countdown reaches zero it re-reads
 * the cell once more and increments the object's byte 2 with a byte-width add,
 * advancing the dispatcher on work byte 2 to its next handler step. The second
 * and third reads of the cell are required: a store through the loaded work
 * pointer may alias the cell, and the named byte offset keeps every record
 * access deriving its own base instead of holding one address register across
 * the read-modify-write, which is the form the original bytes show (lui
 * $at,%hi / addu $at,$at,$v1 for both the load and the store). Leaf with no
 * frame, no saved registers and no calls; takes no arguments, returns nothing
 * and reads or writes no other state. The counter triplet copies
 * incrementWorkByteBRecordCountersAndWorkByte1WhenWorkByte9Is10BmagicMagic
 * 01000_801EED0C, whose statements are the same three byte-width
 * read-modify-writes on this table, and the countdown tail copies
 * decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic09603_801EF394. No
 * payload in the repository holds a byte-identical copy of this body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteARecordCountersThenDecrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic21303_801EF23C(void) {
  u32 offset;

  offset = g_battle_work[0xA] * 12;
  D_80145BD6[offset]++;
  offset = g_battle_work[0xA] * 12;
  D_80145BD7[offset]++;
  offset = g_battle_work[0xA] * 12;
  D_80145BD8[offset]++;

  if (--g_battle_work[9] == 0) {
    g_battle_work[2]++;
  }
}
