#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original reads the cell once at
 * entry (lui $a0,0x1F80 / lw $a0,68($a0)) for every access of the first
 * statement group and re-materializes it (lui $v1,0x1F80 / lw $v1,68($v1))
 * for the byte 0x2 advance, which is the reload the exact siblings
 * incrementWorkByte9AndAddVelocityPairThenAdvanceWorkByte2At18
 * BmagicMagic04303_801EF5E0 and the word-0x40/0x44 motion steps of
 * emi/bmagic/magic008/00 bind the same way. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EF380
 * @behavior Per-frame step of the bmagic work object. It is word 3 (index 3)
 *           of the code-pointer run at 0x801EF6CC that this overlay's
 *           dispatcher dispatchStateByte1Tablef6ccBmagicMagic15403_801EF0C0
 *           indexes with work-object byte 0x1, the run that holds
 *           0x801EF104, 0x801EF4B4, 0x801EF1EC, this address, 0x801EF3E4 and
 *           0x801EF454. It reads the scratchpad work-object pointer cell at
 *           0x1F800044 once, loads the object's countdown byte 0x9, advances it
 *           by one with a byte-width add (lbu / addiu 1 / sb) and keeps the
 *           incremented value in a byte local so the truncation, not a reload,
 *           supplies the compared value (andi before the test). In the same
 *           pointer it advances the 16.16 motion pair: the word at 0x40 gains
 *           0x4000 with an immediate add (lw / addiu / sw) and the word at 0x44
 *           gains 0x8000, which is materialized with ori because it does not
 *           fit the signed immediate (lw / addu / sw); both sums are written
 *           back with sw in the original order and the 0x40 store lands before
 *           the byte-0x9 store. These are the 0x40/0x44 words the exact
 *           siblings incrementWorkCountdownThenSeedMotionPairAndSetTintFromWork
 *           ByteBAt4ThenAdvanceWorkByte2BmagicMagic01500_801F0654 seed with
 *           0x8000/0x18000 and the emi/bmagic/magic008/00 motion steps add to or
 *           subtract from, so this function continues an existing per-frame
 *           velocity update rather than establishing the pair. When the new
 *           byte 0x9 equals four it re-reads the pointer cell and increments the
 *           object's byte 0x2 with a byte-width add (lbu / addiu 1 / sb),
 *           advancing the work byte 0x2 step index; below four it falls through
 *           to the join and only returns. The function is a leaf with no frame,
 *           returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceMotionPairThenAdvanceWorkByte2At4BmagicMagic15403_801EF380(void) {
  u8 *work;
  u8 counter;
  s32 word40;
  s32 word44;

  work = D_1F800044;
  counter = work[9] + 1;
  word40 = *(s32 *)(work + 0x40) + 0x4000;
  word44 = *(s32 *)(work + 0x44) + 0x8000;
  work[9] = counter;
  *(s32 *)(work + 0x40) = word40;
  *(s32 *)(work + 0x44) = word44;
  if (counter == 4) {
    D_1F800044[2]++;
  }
}
