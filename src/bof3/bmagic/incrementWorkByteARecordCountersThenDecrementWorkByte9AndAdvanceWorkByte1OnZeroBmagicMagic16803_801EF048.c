#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable scratchpad,
 * outside every payload. The compiled original re-materialises and reloads the
 * cell (lui $v0,0x1F80 / lw $v0,0x44($v0)) before each of its five accesses,
 * because the stores this body makes may alias the word, so the cell is reached
 * through this symbol rather than through a constant address, which the
 * compiler materialises once and reuses. No map row of this target names the
 * word, so it stays address-anchored here, the same spelling this target's
 * exact lift of 0x801EF158 uses. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Counter bytes 0x0, 0x1 and 0x2 of the 12-byte records this handler advances.
 * The records are selected by the work-object unsigned byte 0xA times the
 * 12-byte stride, and the original reaches the record base and its +2 byte
 * through their own addresses (lui $at,%hi / addu $at,$at,$v1 before both the
 * load and the store of each pair). The middle counter has no symbol of its
 * own, so it is reached through the record base plus one, which is the
 * addressing the original emits (%hi(D_80145BD6 + 0x1) /
 * %lo(D_80145BD6 + 0x1)). The bytes at 0x80145BD6 and 0x80145BD8 also arrive
 * from this target's generated undefined_syms_auto.txt. */
extern u8 D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern u8 D_80145BD8[]; /* @source 0x80145BD8 @kind table */

/* @source 0x801EF048
 * @behavior Handler at entry 2, zero-based, of the six-entry code pointer run
 *           at 0x801EEC04: this overlay's entry dispatcher func_801EEC28 copies
 *           the six words at 0x801EEC04 into its frame and calls the word the
 *           work-object byte at offset 1 scales by four, and this address is
 *           word 2 of that run. It reads the scratchpad work-object pointer
 *           cell at 0x1F800044 once per counter group and, for each of the
 *           record's three counter bytes, re-reads the work object byte 0xA,
 *           scales it by the 12-byte record stride into a byte offset and
 *           increments the selected record byte 0x0, byte 0x1 and the byte at
 *           0x80145BD8 with a byte-width read-modify-write (lbu / addiu
 *           $v0,$v0,1 / sb). It then re-reads the cell, and the work-object
 *           byte 9 with it, and decrements that byte in the same byte-width
 *           form before testing the stored value against zero (andi
 *           $v0,$v0,0xFF / bnez). Only when that countdown reaches zero does it
 *           re-read the cell once more and increment the object byte 1 with a
 *           byte-width add, so the next dispatch selects word 3 of the same
 *           run, the stage arm at 0x801EF158. The second and third reads of the
 *           cell are required: a store through the loaded work pointer may
 *           alias the cell. Leaf with no saved registers and no calls; takes no
 *           arguments, returns nothing and reads or writes no other state.
 *
 *           The original allocates a 0x10-byte frame whose bytes this body
 *           never touches, with the reservation scheduled into the bnez delay
 *           slot and the release before the return. The declared local below
 *           reproduces exactly that reserved size and takes no use; the bytes
 *           constrain only the size, so its name and type are a reconstruction
 *           choice flagged for review, and removing it leaves the body at
 *           0x100 bytes with no frame.
 *
 *           Bytes 0x801EF048..0x801EF157 are byte-identical (0x110 bytes,
 *           proved by a payload cmp of the two spans and by a clone dry run) to
 *           the exact lift of emi/bmagic/magic062/03 at 0x801EEF68, which
 *           supplied the statement order, the operand forms and the semantic
 *           name. That donor reaches the scratchpad cell through its own map
 *           binding, which this target does not have, so the cell here keeps
 *           the address-anchored spelling above; the choice of name does not
 *           reach the bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteARecordCountersThenDecrementWorkByte9AndAdvanceWorkByte1OnZeroBmagicMagic16803_801EF048(void) {
  /* Reserved frame only; no access and no use. Reconstruction choice. */
  u8 scratch[0x10];
  u32 offset;

  offset = D_1F800044[0xA] * 12;
  D_80145BD6[offset]++;
  offset = D_1F800044[0xA] * 12;
  D_80145BD6[offset + 1]++;
  offset = D_1F800044[0xA] * 12;
  D_80145BD8[offset]++;

  if (--D_1F800044[9] == 0) {
    D_1F800044[1]++;
  }
}
