#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
extern s8 D_801F4DEC[]; /* @source 0x801F4DEC @kind table */

/* @source 0x801F3CB0
 * @behavior Steps the work record published through the scratchpad pointer cell
 *           0x1F800044 by one entry of the signed 16-entry phase table at
 *           0x801F4DEC, whose values repeat the pattern -1, 0, 1, 0 and then
 *           -2, 0, 2, 0: while the record phase byte at 0x0A is nonzero it adds
 *           the entry selected by that byte's low nibble, scaled by 0x800, to
 *           the record word at 0x34, subtracts the same scaled entry from the
 *           word at 0x38, decrements the phase byte, and consumes two units of
 *           the halfword at 0x122 of the record published through the global
 *           pointer cell 0x80146250. Once the phase byte reads zero it instead
 *           clears the handler byte at 0x04, which selects the first entry of
 *           the local handler table that dispatches on that byte. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The original lets its test load feed the first table index and re-reads the
 * phase byte for the second index and for the decrement, so those three reads
 * are volatile; the record words and the shared record stay plain because the
 * original lets the scheduler interleave those accesses.
 */
void stepWorkAccumulatorsByPhaseTableConsumeCountdownThenClearHandlerByteWorld01Area06813_801F3CB0(void) {
  volatile u8* work;
  u8* shared;
  s32 step;
  u8 phase;

  work = D_1F800044;
  phase = work[0x0a];
  if (phase != 0) {
    *(s32*)(work + 0x34) += D_801F4DEC[phase & 0x0f] << 11;
    step = D_801F4DEC[work[0x0a] & 0x0f] << 11;
    work[0x0a] = work[0x0a] - 1;
    shared = D_80146250;
    *(s32*)(work + 0x38) -= step;
    *(u16*)(shared + 0x122) -= 2;
  } else {
    work[4] = 0;
  }
}
