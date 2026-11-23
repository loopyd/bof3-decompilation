#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146884; /* @source 0x80146884 @kind unknown */
extern s8 D_801F4A88[]; /* @source 0x801F4A88 @kind table */

/* @source 0x801F3FC0
 * @behavior Per-frame step of the work record published through the
 *           scratchpad pointer cell 0x1F800044, gated on its phase byte at
 *           0x0A. While that byte is set, adds the signed phase-table entry at
 *           0x801F4A88 selected by the low nibble of the phase byte, scaled by
 *           0x800, to the record word at 0x34, subtracts the entry selected by
 *           a fresh read of the same nibble from the word at 0x38, decrements
 *           the phase byte and consumes two units of the halfword at 0x7E of
 *           the record published through 0x80146884. Once the phase byte reads
 *           zero it instead clears the handler byte at 0x04, which selects the
 *           first entry of the local handler table at 0x801F4A80. The overlay
 *           reaches the body through that table as the second entry, so the
 *           parameter roles stay structural: no arguments and no return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The original re-reads the phase byte for the second table index and for the
 * decrement even though no store to that byte separates those reads, so the
 * record is volatile; the record words and the shared record stay plain
 * because the original lets the scheduler interleave those accesses.
 */
void stepWordPhaseConsume(
    void) {
  volatile u8* work;
  u8* shared;
  s32 step;
  u8 phase;

  work = D_1F800044;
  phase = work[0x0a];
  if (phase != 0) {
    *(s32*)(work + 0x34) += D_801F4A88[phase & 0x0f] << 11;
    step = D_801F4A88[work[0x0a] & 0x0f] << 11;
    work[0x0a] = work[0x0a] - 1;
    shared = D_80146884;
    *(s32*)(work + 0x38) -= step;
    *(u16*)(shared + 0x7e) -= 2;
  } else {
    work[4] = 0;
  }
}
