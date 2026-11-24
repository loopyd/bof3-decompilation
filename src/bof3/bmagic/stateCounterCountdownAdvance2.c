#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * compiled original materialises the base of the cell at every access
 * (lui $v0,0x1F80 / lw $v0,0x44($v0)), once per counter group and once per
 * countdown-tail group, so the cell is reached through this address-derived
 * pointer rather than through a constant-address accessor, which the compiler
 * materialises once and reuses. No map row of this target names the word and
 * this target's generated undefined_syms_auto.txt has no row for it either, so
 * it stays address-anchored here. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Counter bytes 0x0 and 0x2 of the 12-byte records this handler advances. The
 * record is selected by the work object's unsigned byte 0xA times the 12-byte
 * stride, and the original reaches the record base and the base+2 byte through
 * their own addresses (lui $at,%hi / addu $at,$at,$v1 emitted for both the load
 * and the store of each pair). 0x80145BD6 and 0x80145BD8 arrive from this
 * target's generated undefined_syms_auto.txt. The middle counter at 0x80145BD7
 * has no symbol of its own, so it is reached through the record base plus one,
 * which is the addressing the original emits (%hi(D_80145BD6 + 0x1) /
 * %lo(D_80145BD6 + 0x1) over the same scaled $at). */
extern u8 D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern u8 D_80145BD8[]; /* @source 0x80145BD8 @kind table */

/* @source 0x801EF04C
 * @behavior Handler at entry 2 (zero-based) of the six-entry code-pointer run
 *           at 0x801EEC04, which this overlay's dispatcher func_801EEC28 copies
 *           into its own 0x38-byte frame and indexes on the work object's byte
 *           at offset 1 with a shifted byte (lbu / sll 2 / addu $sp / lw from
 *           0x10($sp)). That run's word 2 holds 0x801EF04C in this payload; the
 *           six entries are 0x801EED38, 0x801EEFA4, 0x801EF04C, 0x801EF15C,
 *           0x801EF1A4 and 0x801EF2DC. The body reads the scratchpad work-object
 *           pointer cell at 0x1F800044 once per counter group and, for each of
 *           the record's three counter bytes, re-reads the work object's
 *           unsigned byte 0xA, scales it by the 12-byte record stride into a
 *           byte offset and increments the selected record byte with a
 *           byte-width read-modify-write (lbu / addiu $v0,$v0,1 / sb), in the
 *           order byte 0x0, byte 0x1, byte 0x2. It then re-reads the cell, and
 *           the work object's unsigned byte 9 with it, and decrements that byte
 *           in the same byte-width form before testing the stored value against
 *           zero (andi $v0,$v0,0xFF / bnez). Only when that countdown reaches
 *           zero does it re-read the cell once more and increment the object's
 *           byte 1 with a byte-width add, which advances the handler index the
 *           dispatcher selects on to the next entry of the same run. The second
 *           and third reads of the cell are emitted as written: a store through
 *           the loaded work pointer may alias the cell. Leaf with no saved
 *           registers and no calls; takes no arguments, returns nothing and
 *           reads or writes no other state.
 *
 *           The original reserves a 0x10-byte frame whose bytes this body never
 *           touches, with the reservation scheduled into the bnez delay slot
 *           and the release before the return. The declared local below
 *           reproduces exactly that reserved size and takes no use; the bytes
 *           constrain only the size, so its name and type are a reconstruction
 *           choice flagged for review, and removing it leaves the body at
 *           0x100 bytes with no frame.
 *
 *           Evidence for the statement order, operand forms and declaration
 *           style: the 0x110-byte span 0x801EF04C..0x801EF15B is byte-identical
 *           (payload cmp, md5 bd9c95c37473cea76b26ac1c391ed797 on both 272-byte
 *           extracts) to the already-exact emi/bmagic/magic070/03 lift at
 *           0x801EF04C and to the already-exact emi/bmagic/magic062/03 lift at
 *           0x801EEF68, both re-proved against this payload rather than copied
 *           from their comments. Every literal, cell access, offset and shift
 *           count here comes from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCounterCountdownAdvance2(
    void) {
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
