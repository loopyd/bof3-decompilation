#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
extern s8 D_801F4C84[]; /* @source 0x801F4C84 @kind table */

/* @source 0x801F2EA8
 * @behavior Per-frame phase step of the work record published through the
 *           scratchpad pointer cell 0x1F800044, gated on its phase byte at
 *           0x0A. While that byte is set it adds the signed entry of the local
 *           16-entry phase table at 0x801F4C84 selected by the low nibble of
 *           the phase byte, scaled by 0x800, to the record word at 0x34,
 *           subtracts the entry selected by a fresh read of the same nibble
 *           from the word at 0x38, decrements the phase byte and consumes two
 *           units of the halfword at 0x122 of the record published through the
 *           global pointer cell 0x80146250. Once the phase byte reads zero it
 *           instead clears the handler byte at 0x04, which selects entry 0 of
 *           the local handler table at 0x801F4C7C, the armer at 0x801F2E68 that
 *           stores 0x40 into that phase byte and 1 into that handler byte. Takes
 *           no arguments, makes no call, keeps no frame and returns nothing.
 *
 * Name evidence: the only word of this payload naming 0x801F2EA8 is entry 1 of
 *   the handler table at 0x801F4C7C, whose bytes hold 0x801F2E68 and
 *   0x801F2EA8, and the dispatcher at 0x801F2E24 indexes that table with the
 *   record byte at 0x04 after the entry-0 armer has stored 1 there. The table
 *   read here, 0x801F4C84, is this payload's own signed 16-entry phase table
 *   whose bytes repeat -1, 0, 1, 0 through the payload's last 16 bytes, and
 *   each entry is scaled by 0x800, so the name states the per-phase step the
 *   record word pair takes and the handler byte cleared when it runs out.
 *
 * Shape evidence: a word comparison of this 36-word span against the extracted
 *   bodies found no byte-identical copy, so the clone lever is declined. The
 *   nearest relative is the world01 area068 step at 0x801F3CB0, whose span
 *   differs in exactly three words: the two %lo halves of the load that reads
 *   its own phase table at 0x801F4DEC, and the absolute j word, which shifts by
 *   0xE08, the address delta between the two spans. Every other word, the
 *   shared pointer cell 0x80146250 and the halfword offset 0x122 included, is
 *   equal, so that body supplied the statement order, declaration style and
 *   offset spelling while every literal and binding here comes from this span's
 *   own bytes. The same body shape is lifted in world01 area039 at 0x801F2C88
 *   against the pointer cell 0x80146884 and the halfword offset 0x7E.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The original re-reads the phase byte for the second table index and for the
 * decrement even though no store to that byte separates those reads, so the
 * record is volatile; the record words and the shared record stay plain
 * because the original lets the scheduler interleave those accesses.
 */
void stateStepPhaseWords(
    void) {
  volatile u8* work;
  u8* shared;
  s32 step;
  u8 phase;

  work = D_1F800044;
  phase = work[0x0a];
  if (phase != 0) {
    *(s32*)(work + 0x34) += D_801F4C84[phase & 0x0f] << 11;
    step = D_801F4C84[work[0x0a] & 0x0f] << 11;
    work[0x0a] = work[0x0a] - 1;
    shared = D_80146250;
    *(s32*)(work + 0x38) -= step;
    *(u16*)(shared + 0x122) -= 2;
  } else {
    work[4] = 0;
  }
}
