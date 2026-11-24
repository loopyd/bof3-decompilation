#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This target's map carries no row for
 * 0x1F800044 and the D_<ADDRESS> spelling auto-binds the word, so the source
 * declares it here. The original re-materialises the base at each access
 * (lui $v0,0x1F80 / lw $v0,0x44($v0)), once per counter group and once per
 * countdown-tail group, so the cell stays a named pointer rather than a
 * constant-address accessor. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Counter bytes 0x0 and 0x2 of the 12-byte records this handler advances. The
 * records are selected by the work object's unsigned byte 0xA times the
 * 12-byte stride, and the original reaches the record base and its +2 byte
 * through their own addresses (lui $at,%hi / addu $at,$at,$v1 before both the
 * load and the store of each pair). The bytes at 0x80145BD6 and 0x80145BD8
 * arrive from this target's generated undefined_syms_auto.txt. The middle
 * counter at 0x80145BD7 has no symbol of its own in this target, so it is
 * reached through the record base plus one, which is the addressing the
 * original emits (%hi(D_80145BD6 + 0x1) / %lo(D_80145BD6 + 0x1)). */
extern u8 D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern u8 D_80145BD8[]; /* @source 0x80145BD8 @kind table */

/* @source 0x801EF04C
 * @behavior Handler at entry 2, zero-based, of the six-entry code pointer
 *           table at 0x801EEC04 that this overlay's work-byte-1 dispatcher at
 *           0x801EEC28 copies to its own frame and indexes on the work-object
 *           byte at offset 1. It reads the scratchpad work-object pointer cell
 *           at 0x1F800044 once per counter group and, for each of the record's
 *           three counter bytes, re-reads the work object's unsigned byte 0xA,
 *           scales it by the 12-byte record stride into a byte offset and
 *           increments the selected 0x80145BD6 record's bytes 0x0, 0x1 and 0x2
 *           with a byte-width read-modify-write (lbu / addiu $v0,$v0,1 / sb).
 *           It then re-reads the cell, and the work object's unsigned byte 9
 *           with it, and decrements that byte in the same byte-width form
 *           before testing the stored value against zero (andi $v0,$v0,0xFF /
 *           bnez). Only when that countdown reaches zero does it re-read the
 *           cell once more and increment the object's byte 1 with a byte-width
 *           add, advancing the handler index the 0x801EEC28 dispatcher selects
 *           on to the next entry of the same run. The second and third reads of
 *           the cell are required: a store through the loaded work pointer may
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
 *           The 0x110-byte span 0x801EF04C..0x801EF15B is byte-identical to the
 *           emi/bmagic/magic062/03 span at 0x801EEF68..0x801EF077 (proved by a
 *           payload cmp of the two 0x110-byte ranges), so the clone lever
 *           supplied the statement order, the declaration style and the frame
 *           reconstruction above; the pointer cell was re-spelled from that
 *           target's map name to this one's D_<ADDRESS> binding, and every
 *           literal, cell access and offset here comes from this span's own
 *           bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCounterCountdownAdvance(void) {
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
