#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146884; /* @source 0x80146884 @kind unknown */
extern s8 D_801F4990[]; /* @source 0x801F4990 @kind table */

/* @source 0x801F2DEC
 * @behavior Steps the work record published through the scratchpad pointer cell
 * at 0x1F800044 by one entry of the signed 16-entry phase table at 0x801F4990,
 * whose values repeat the pattern -1, 0, 1, 0: it subtracts the entry selected
 * by the low nibble of the record's phase byte at 0x0A, scaled by 0x800, from
 * the word at 0x38, decrements that phase byte, and consumes two units of the
 * halfword at 0x7E of the record published through the global pointer cell
 * 0x80146884. Once the phase byte reads zero it instead clears the handler byte
 * at 0x04, which selects the first entry of the local handler table at
 * 0x801F4988 that dispatches on that byte. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The original re-reads the phase byte for the decrement even though no store
 * separates that read from the one its own test uses, so the second read is
 * volatile; the record words and the shared record stay plain because the
 * original lets the scheduler interleave those accesses.
 */
void stepWorkAccumulatorByPhaseTableConsumeCountdownThenClearHandlerByteWorld02Area09913_801F2DEC(void) {
  volatile u8* work;
  u8* shared;
  s32 step;
  u8 phase;

  work = D_1F800044;
  phase = work[0x0a];
  if (phase != 0) {
    step = D_801F4990[phase & 0x0f] << 11;
    work[0x0a] = work[0x0a] - 1;
    shared = D_80146884;
    *(s32*)(work + 0x38) -= step;
    *(u16*)(shared + 0x7e) -= 2;
  } else {
    work[4] = 0;
  }
}
