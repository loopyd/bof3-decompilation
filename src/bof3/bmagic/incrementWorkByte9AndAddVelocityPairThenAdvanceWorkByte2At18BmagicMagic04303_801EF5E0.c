#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * by the battle lifts. The original re-materializes the cell address (lui
 * $vN,0x1F80 / lw $vN,68($vN)) for both of the reads in this function, so the
 * cell is declared volatile like the exact sibling
 * decrementMotionCountdownAndSubtractMotionPairThenAdvanceWorkByte2
 * BmagicMagic00800_801F04DC. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF5E0
 * @behavior Per-frame step of the work-object handler this overlay dispatches
 * on work-object byte 0x1: it is word 3 of the five-word handler run at
 * 0x801EFC88 (0x801EF3A4, 0x801EF42C, 0x801EF55C, 0x801EF5E0, 0x801EF64C) that
 * 0x801EF360 indexes with that byte, and word 2 of the four-word run at
 * 0x801EFC8C that 0x801EF3A4 indexes with byte 0x2. It reads the scratchpad
 * work-object pointer cell at 0x1F800044 once, loads that object's unsigned
 * byte 0x9 and advances it by one with a byte-width add (lbu / addiu 1 / sb),
 * adds the words at 0xC and 0x10 into the words at 0x34 and 0x38 with two
 * addu and writes both sums back with sw in the original order, so the object's
 * word-0x34 and word-0x38 channels gain their word-0xC and word-0x10 partners
 * (the pair 0x801EF42C seeds with 0x20000 and zero, the shape the sibling
 * copySharedXToWorkXIfInRangeWorld04Area17513_801F3FDC treats as the X
 * channel). When the new byte 0x9 equals 0x18 it re-reads the pointer cell and
 * increments the object's byte 0x2 with a byte-width add (lbu / addiu 1 / sb),
 * advancing to the next byte-0x2 handler. The function is a leaf with no frame;
 * it returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAddVelocityPairThenAdvanceWorkByte2At18BmagicMagic04303_801EF5E0(void) {
  u8 *work;
  u8 counter;
  s32 channelX;
  s32 channelZ;

  work = g_battle_work;
  counter = work[9] + 1;
  channelX = *(s32 *)(work + 0x34) + *(s32 *)(work + 0xC);
  channelZ = *(s32 *)(work + 0x38) + *(s32 *)(work + 0x10);
  work[9] = counter;
  *(s32 *)(work + 0x34) = channelX;
  *(s32 *)(work + 0x38) = channelZ;
  if (counter == 0x18) {
    g_battle_work[2]++;
  }
}
