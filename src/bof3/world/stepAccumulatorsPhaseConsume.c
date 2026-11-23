#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
extern s8 D_801F4DD4[]; /* @source 0x801F4DD4 @kind table */

/* @source 0x801F3B9C
 * @behavior Phase mover at index 1 of the byte-4 dispatch table at 0x801F4DCC,
 *           whose index 0 names the armer at 0x801F3B5C: it reads the work
 *           record published through the scratchpad pointer cell 0x1F800044 and
 *           tests its phase byte at 0x0A. If that byte is nonzero it adds the
 *           signed entry that byte's low nibble selects from the local phase
 *           table at 0x801F4DD4, whose sixteen bytes repeat -1, 0, 1, 0, scaled
 *           by 0x800, to the record word at 0x34, subtracts the same scaled
 *           entry from the word at 0x38, stores the phase byte minus one,
 *           re-reading the phase byte for the second table index and for the
 *           decrement, and consumes two units of the halfword at 0x122 of the
 *           shared work record published at 0x80146250. If the phase byte reads
 *           zero it instead clears the handler byte at 0x04, so the next
 *           dispatch of the same table reaches the armer at index 0 again. Leaf:
 *           no call, no frame, takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepAccumulatorsPhaseConsume(void) {
  volatile u8* work;
  u8* shared;
  s32 step;
  s32 value;
  u8 phase;
  u8 nextIndex;

  work = D_1F800044;
  phase = work[0x0a];
  if (phase != 0) {
    step = D_801F4DD4[phase & 0x0f] * 2048;
    value = *(volatile s32*)(work + 0x34);
    nextIndex = work[0x0a] & 0x0f;
    *(volatile s32*)(work + 0x34) = value + step;
    step = D_801F4DD4[nextIndex] * 2048;
    work[0x0a] = work[0x0a] - 1;
    shared = D_80146250;
    *(volatile s32*)(work + 0x38) -= step;
    *(u16*)(shared + 0x122) -= 2;
  } else {
    work[4] = 0;
  }
}
