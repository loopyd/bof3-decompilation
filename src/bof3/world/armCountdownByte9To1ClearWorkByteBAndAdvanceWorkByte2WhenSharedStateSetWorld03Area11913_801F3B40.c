#include "bof3/bof3.h"

extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* @source 0x801F3B40
 * @behavior Overlay step handler: when the shared overlay state byte at
 * 0x8014933B is nonzero it arms the work record's countdown byte at offset 0x09
 * with 1, clears the record's byte at offset 0x0B to 0 and advances the record's
 * state byte at offset 0x02 by one, so the byte-2 dispatch table moves on to the
 * next handler. The work record comes from the scratchpad pointer cell
 * 0x1F800044, which is re-read before each of the three store groups. Takes no
 * arguments, carries no frame and returns nothing.
 * This is the first entry of the handler table at 0x801F460C used by the byte-2
 * dispatcher dispatchStateByte2Table460cWorldWorld03Area11913_801F3AFC: the
 * payload word at 0x801F460C holds 0x801F3B40 and the next word holds the
 * following handler at 0x801F3B98, which indexes its own offset-0x0B byte into
 * the eight-entry position table at 0x801F4614 and increments it modulo 8. The
 * constant 1 stored to the countdown byte fills the guard branch's delay slot in
 * the original. The store shape follows the exact sibling
 * resetScratchStateToMode1World03Area13413_801F35A4, which re-reads the same
 * pointer cell before each of its three byte stores, and the countdown role of
 * offset 0x09 follows this overlay's countdownScratchStateToMode3World03Area11913_801F2CE0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdownByte9To1ClearWorkByteBAndAdvanceWorkByte2WhenSharedStateSetWorld03Area11913_801F3B40(void) {
  u8** slots;
  u8 value;

  if (D_8014933B != 0) {
    slots = SPAD_PTR_TABLE(u8);
    value = 1;
    slots[0x11][0x09] = value;
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x0B] = 0;
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x02] += 1;
  }
}
