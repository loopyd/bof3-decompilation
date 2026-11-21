#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, the form this target's other lifts use
 * (integrateWorkWord14IntoHalfword3EAndDecrementByte9ThenInvokeHelpersAndAdvanceAtZeroBmagicMagic00100_801EF280
 * and
 * subtractWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWorkByte2WhenChannelIs80BmagicMagic00100_801EF5CC).
 * The original re-reads the cell in the taken arm after the stores through the
 * previously loaded pointer, because those stores may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EF408
 * @behavior Handler registered as word 9 of the overlay's code pointer run at
 *           0x801EEC04 (its own word sits at 0x801EEC28). Reads the scratchpad
 *           work object through cell 0x1F800044 once, decrements the work
 *           object unsigned byte 0xA, and integrates motion: it adds work word
 *           0x20 into work word 0x14, then adds the new word 0x14 into the work
 *           halfword 0x3E. The three stores go out in the order byte 0xA, word
 *           0x14, halfword 0x3E, and the countdown test is on the decremented
 *           byte, taken modulo 256. When that byte reaches zero it re-reads the
 *           cell and advances work byte 2, the handler index, with a byte-width
 *           read-modify-write. Leaf with no frame; takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkWord20ToWord14AccumulateIntoHalfword3EAndDecrementByteAThenAdvanceWorkByte2AtZeroBmagicMagic00100_801EF408(void) {
  u8 *work;
  u8 countdown;
  u32 value;
  u16 accumulator;

  work = D_1F800044;
  countdown = work[0xA] - 1;
  value = *(u32 *)(work + 0x14) + *(u32 *)(work + 0x20);
  accumulator = *(u16 *)(work + 0x3E) + value;
  work[0xA] = countdown;
  *(u32 *)(work + 0x14) = value;
  *(u16 *)(work + 0x3E) = accumulator;
  if (countdown == 0) {
    D_1F800044[2]++;
  }
}
