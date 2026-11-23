#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol rather than
 * through SPAD_PTR_SLOT so each access re-materialises the cell address with
 * its own lui, which is what the original does before every store group; the
 * bare constant address is instead hoisted once into a base register. The
 * sibling seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * spells the same cell this way. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared no-argument update helper of the main executable, outside this
 * payload. The original sets nothing up in the argument registers and reads no
 * result from $v0, so the prototype is spelled without an address tag, as the
 * plchar lifts that call it spell it. */
void func_8014D978(void);

/* Representation note from the two measured spellings of this body: holding the
 * tail stores in the same pointer variable as the countdown group measured
 * 23/27 instructions (85.19%) because that one temporary took $a0 for both
 * groups, while the original reloads the cell into $v1 for the countdown and
 * into $a0 for the tail. Giving the tail group its own pointer temporary, as
 * the sibling seedWorkMotionFromBattlerRecordAndAdvanceWorkByte2 here spells
 * its 0x18/0x1C pair, measured 27/27 over the same 108 bytes. */
/* @source 0x801EF43C
 * @behavior Work-byte-2 handler, entry 3 (index 2) of the six-entry
 *           handler-pointer run at 0x801EEC14 (0x801EF238, 0x801EF3A4,
 *           0x801EF43C, 0x801EF4A8, 0x801EF534, 0x801EF624) that the overlay
 *           dispatcher at 0x801EF19C copies into its own frame and selects
 *           with byte 2 of the scratchpad work object at 0x1F800044. It first
 *           runs the shared no-argument update helper 0x8014D978, then
 *           decrements work byte 9 by one at byte width and stores it before
 *           testing it. When the result is zero it publishes the acceleration
 *           pair at work words 0x18 and 0x1C as 0x60 and -0x80, the same pair
 *           the sibling seedWorkMotionFromBattlerRecordAndAdvanceWorkByte2
 *           BmagicMagic05403_801EF238 writes, and advances work byte 2. It
 *           re-reads the pointer cell after the byte-9 store because that store
 *           through the loaded pointer may alias the cell, and it keeps the
 *           decremented value for the zero test rather than reloading the byte.
 *           The 0x18-byte frame only keeps $ra at 0x10 across the helper call;
 *           it takes no arguments, returns nothing and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSharedUpdateThenDecrementWorkByte9AndSetAccelerationPairWhenZeroBmagicMagic05403_801EF43C(
    void) {
  u8 *work;
  u8 *velocity;
  u8 remaining;

  func_8014D978();
  work = D_1F800044;
  remaining = work[9] - 1;
  work[9] = remaining;
  if (remaining == 0) {
    velocity = D_1F800044;
    *(u32 *)(velocity + 0x18) = 0x60;
    *(u32 *)(velocity + 0x1C) = -0x80;
    velocity[2]++;
  }
}
