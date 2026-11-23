#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Every store through the record re-reads
 * this cell, because a byte store through the record may alias the cell. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Record published through this global pointer cell; this body consumes two
 * units of its halfword at 0x122. */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */

/* Shared work-record step at 0x801F4118: adds the work-record step word at
 * 0x0C into the accumulator word at 0x34 and adds 2 to each of the three
 * counter bytes of the 12-byte record indexed by the byte at 0x13D of the
 * record published through 0x80146250. */
extern void
stateAccumulatorCounterTriple(
    void);

/* @source 0x801F3E80
 * @behavior Overlay work-record handler, entry 0 of the state-byte-4 handler
 * table based at 0x801F567C: stores 0x4000 into the work-record step word at
 * 0x0C, runs the shared accumulator step at 0x801F4118 so that the value lands
 * in the accumulator word at 0x34, then arms the countdown byte at 0x0A with 7
 * and the state byte at 0x04 with 1, the index of the next handler in that
 * table. Two units of the halfword at 0x122 of the record published through
 * 0x80146250 are consumed last. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateAccumulatorEnterConsume(void) {
  s32* words;
  u8* shared;

  words = (s32*)D_1F800044;
  words[3] = 0x4000;
  stateAccumulatorCounterTriple();
  D_1F800044[0x0a] = 7;
  D_1F800044[4] = 1;
  shared = D_80146250;
  *(u16*)(shared + 0x122) -= 2;
}
