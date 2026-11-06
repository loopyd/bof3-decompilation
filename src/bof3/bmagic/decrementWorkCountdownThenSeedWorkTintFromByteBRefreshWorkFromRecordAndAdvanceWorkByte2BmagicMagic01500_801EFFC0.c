#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. The sibling bmagic lifts read the same
 * record: seedWorkWordsAndCopyRecordByte27ThenAdvanceWorkByte2BmagicMagic01500
 * _801F0D48 copies its byte 0x27 and the triplet of 0x34/0x38/0x3C words this
 * function copies, and seedWorkMotionFieldsWhenGlobalWorkByteBIs1Bmagic
 * Magic01500_801EFD7C and decrementWorkCountdownThenSeedMotionFieldsAnd
 * SetTintWhenGlobalWorkByteBIsSetBmagicMagic01500_801F045C guard on its byte
 * 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler byte in main RAM, outside this overlay's image. The battle
 * targets own the address; this lift only reads it to separate a party battler
 * (below three) from an enemy, exactly as the sibling
 * advanceWorkByte2ByPartyOrEnemyStepThenCopyRecordByte27SeedByte2BAnd
 * CopyRecordTripletBmagicMagic01500_801EFAE4 does when it advances the same
 * work byte 2 by one or by two. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before each store group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EFFC0
 * @behavior Entry 20 of this overlay's header handler table, the code-pointer
 *           run that starts at 0x801EEC04 and stores this function's address at
 *           0x801EEC54, between invokeHelperBmagicMagic01500_801EFEFC and
 *           func_801F014C. It reads the scratchpad work-object pointer cell at
 *           0x1F800044 once, decrements the object's countdown byte 9 and
 *           stores it back with sb, keeping the decremented value in a byte
 *           local so the truncation, not a reload, supplies the compared
 *           value. While the new countdown is nonzero it only returns. When the
 *           countdown reaches zero it re-reads the cell and raises bit 0x20 of
 *           work byte 0 (the flag seedMotionTintAndChannelFieldsThenAdvance
 *           WorkByte2BmagicMagic00800_801F05FC sets with its own seed), re-reads
 *           the cell and sets mode byte 0x5C to 1, then sets the signed tint
 *           triplet at 0x5D, 0x5E and 0x5F to -20 * (work byte 0xB + 1), each
 *           channel re-reading the pointer cell and work byte 0xB because a
 *           store through the loaded pointer may alias the cell. It then clears
 *           work byte 0x2B, copies byte 0x27 of the shared battle work record
 *           read through 0x801EB4E0 into work byte 0x27, and advances work byte
 *           2, the handler index, by one when the acting battler byte 0x80146374
 *           is a party member (below three) and by two when it is an enemy,
 *           storing the new value once after the branch with each arm
 *           re-materializing the cell. Finally it copies words 0x34, 0x38 and
 *           0x3C of that record into the same work-object offsets through one
 *           shared cell load, which is why only this last group does not
 *           re-read the cell between the three stores. Leaf, no frame, nothing
 *           returned; it takes no arguments and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkCountdownThenSeedWorkTintFromByteBRefreshWorkFromRecordAndAdvanceWorkByte2BmagicMagic01500_801EFFC0(void) {
  u8 *work;
  u8 countdown;
  u8 *object;
  u8 *mode;
  u8 *source;
  u8 *copyByte;
  u8 *step;
  u8 next;
  u8 *record;
  u8 *copy;

  work = D_1F800044;
  countdown = work[9] - 1;
  work[9] = countdown;
  if (countdown == 0) {
    object = D_1F800044;
    object[0] |= 0x20;
    mode = D_1F800044;
    mode[0x5C] = 1;
    ((s8 *)D_1F800044)[0x5D] = -((((u8 *)D_1F800044)[0xB] + 1) * 20);
    ((s8 *)D_1F800044)[0x5E] = -((((u8 *)D_1F800044)[0xB] + 1) * 20);
    ((s8 *)D_1F800044)[0x5F] = -((((u8 *)D_1F800044)[0xB] + 1) * 20);
    ((u8 *)D_1F800044)[0x2B] = 0;
    source = D_801EB4E0;
    copyByte = D_1F800044;
    copyByte[0x27] = source[0x27];
    if (D_80146374 < 3u) {
      step = D_1F800044;
      next = step[2] + 1;
    } else {
      step = D_1F800044;
      next = step[2] + 2;
    }
    step[2] = next;
    record = D_801EB4E0;
    copy = D_1F800044;
    *(u32 *)(copy + 0x34) = *(u32 *)(record + 0x34);
    *(u32 *)(copy + 0x38) = *(u32 *)(record + 0x38);
    *(u32 *)(copy + 0x3C) = *(u32 *)(record + 0x3C);
  }
}
