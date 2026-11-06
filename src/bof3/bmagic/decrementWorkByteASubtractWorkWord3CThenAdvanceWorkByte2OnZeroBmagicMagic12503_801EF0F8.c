#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and read by every handler of this overlay. The original re-materialises the
 * cell address for the second read (lui $v1,0x1F80 / lw $v1,68($v1)) because a
 * store through the loaded object may alias the cell, so the cell is kept as a
 * named symbol rather than the constant-address memory accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0F8
 * @behavior Per-frame step of the work-object handler this overlay dispatches on
 * work-object byte 1: it is entry 3 of the eleven-entry code-pointer run at
 * 0x801F05D8 (0x801EEF04, 0x801EF808, 0x801EEFF8, 0x801EF0F8, 0x801EF148,
 * 0x801EF890, 0x801EF960, 0x801EFAB0, 0x801EFB8C, 0x801EFCE4, 0x801EFE8C),
 * whose dispatcher is dispatchStateByte1Table05d8BmagicMagic12503_801EEEC0. It
 * reads the scratchpad work-object pointer cell at 0x1F800044 once and then
 * works on the object in place: it decrements byte 0xA with a byte-width
 * read-modify-write (lbu / addiu -1 / sb) and subtracts the encoded 16.16 step
 * 0x800000 (128.0) from the word at 0x3C (lw / addu / sw), whose store the
 * original schedules into the branch delay slot. Only when the new byte 0xA is
 * zero does it re-read the cell and increment byte 2 with a byte-width add,
 * advancing the dispatcher to the next byte-1 handler step. The second read of
 * the cell is required: the stores through the loaded work pointer may alias
 * the cell. Takes no arguments, returns nothing, keeps no frame and reads or
 * writes no other state. The word at 0x3C is the third word of the position
 * triplet 0x34/0x38/0x3C that copyWorkWordTriplet* lifted siblings copy from
 * the battle record, and the step magnitude is the 16.16 encoding of 128.0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteASubtractWorkWord3CThenAdvanceWorkByte2OnZeroBmagicMagic12503_801EF0F8(void) {
  u8 *work;
  u8 counter;
  u32 position;

  work = g_battle_work;
  counter = work[0xA] - 1;
  position = *(u32 *)(work + 0x3C) - 0x800000;
  work[0xA] = counter;
  *(u32 *)(work + 0x3C) = position;
  if (counter == 0) {
    g_battle_work[2]++;
  }
}
