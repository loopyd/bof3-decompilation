#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. The countdown test and the state byte
 * store each re-read this cell, because a byte store through the record may
 * alias it. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Record published through this global pointer cell; this body consumes two
 * units of its halfword at 0x122. */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */

/* Shared work-record step at 0x801F4118: adds the work-record step word at
 * 0x0C into the accumulator word at 0x34 and adds 2 to each of the three
 * counter bytes of the 12-byte record indexed by the byte at 0x13D of the
 * record published through 0x80146250. */
extern void
advanceWorkAccumulatorAndAdd2ToSharedCounterTripleWorld01Area04113_801F4118(
    void);

/* @source 0x801F3EE4
 * @behavior Overlay work-record handler, entry 1 of the state-byte-4 handler
 * table based at 0x801F567C: runs the shared accumulator step at 0x801F4118 so
 * that the record step word at 0x0C lands in the accumulator word at 0x34 and
 * the shared counter triple advances by 2, then decrements the countdown byte
 * at 0x0A of the record published at the scratchpad pointer cell 0x1F800044.
 * When that countdown byte reads zero the handler re-arms it with 8 and stores
 * 2, the index of the next handler in that table, into the record state byte
 * at 0x04. Both paths then consume two units of the halfword at 0x122 of the
 * record published through the global pointer cell 0x80146250. The record is
 * reached through one byte view for the countdown update, and the pointer cell
 * is re-read for the countdown test and again for the state byte because the
 * intervening stores through the old record pointer may alias it. Takes no
 * arguments and returns nothing; the call is the only reason for the 0x18
 * frame that saves the return address at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkAccumulatorAndRearmCountdownThenEnterHandlerTwoOrConsumeSharedCountdownWorld01Area04113_801F3EE4(void) {
  u8* words;
  u8* shared;

  advanceWorkAccumulatorAndAdd2ToSharedCounterTripleWorld01Area04113_801F4118();
  words = D_1F800044;
  words[0x0a] = words[0x0a] - 1;
  if (D_1F800044[0x0a] == 0) {
    D_1F800044[0x0a] = 8;
    D_1F800044[4] = 2;
  }
  shared = D_80146250;
  *(u16*)(shared + 0x122) -= 2;
}
