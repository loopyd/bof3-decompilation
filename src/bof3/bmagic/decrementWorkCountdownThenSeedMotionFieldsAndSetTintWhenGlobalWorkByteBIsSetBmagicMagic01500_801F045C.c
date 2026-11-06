#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. The sibling bmagic lifts read the same
 * record: seedWorkWordsAndCopyRecordByte27ThenAdvanceWorkByte2BmagicMagic01500
 * _801F0D48 copies its byte 0x27, and
 * seedWorkMotionFieldsWhenGlobalWorkByteBIs1BmagicMagic01500_801EFD7C and
 * decrementMotionCountdownAdjustMotionPairThenDecrementGlobalWorkByteBAnd
 * AdvanceWorkByte2BmagicMagic01500_801EFE84 guard on its byte 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before each store group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F045C
 * @behavior Entry 25 of this overlay's header handler table, the code-pointer
 *           run that starts at 0x801EEC04 and stores this function's address at
 *           0x801EEC68. It reads the scratchpad work-object pointer cell at
 *           0x1F800044 once, decrements the object's countdown byte 9 and
 *           stores it back with sb, keeping the decremented value in a byte
 *           local so the truncation, not a reload, supplies the compared value.
 *           While the new countdown is nonzero it only returns. When the
 *           countdown reaches zero it refreshes the work object from the shared
 *           battle work record read through 0x801EB4E0: record byte 0x27 is
 *           copied into work byte 0x27, and work byte 0x2B is set to one when
 *           record byte 0xB is zero; then, when record byte 0xB is nonzero, it
 *           raises bit 0x20 of work byte 0, sets mode byte 0x5C to 1 and sets
 *           the signed tint triplet at 0x5D, 0x5E and 0x5F to -0x40 (0xC0
 *           unsigned), the endpoint addWorkColorTripletBy16ThenDecrementGlobal
 *           WorkByteBAndAdvanceWorkByte2BmagicMagic01500_801F1180 advances to
 *           and seedMotionTintAndChannelFieldsThenAdvanceWorkByte2Bmagic
 *           Magic00800_801F05FC starts below. It then seeds the object: word
 *           0x14 = 0x80, word 0x20 = -2, the 16.16 motion pair at 0x40 and 0x44
 *           = 0x10000, byte 0x48 = 2, re-seeds countdown byte 9 with 0x10 and
 *           finally advances work byte 2, the handler index, with a byte-width
 *           read-modify-write. Each store group reads the pointer cell through
 *           its own local because a store through the loaded pointer may alias
 *           the cell; the shared seed group takes the cell load the tint arm
 *           ends with when record byte 0xB is nonzero, and the guard's own load
 *           when it is zero. Leaf, no frame, nothing returned; it takes no
 *           arguments and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkCountdownThenSeedMotionFieldsAndSetTintWhenGlobalWorkByteBIsSetBmagicMagic01500_801F045C(void) {
  u8 *work;
  u8 countdown;
  u8 *source;
  u8 *copyByte;
  u8 *flag;
  u8 *mode;
  s8 *tint;
  u8 *setCountdown;
  u8 *step;

  work = D_1F800044;
  countdown = work[9] - 1;
  work[9] = countdown;
  if (countdown == 0) {
    source = D_801EB4E0;
    copyByte = D_1F800044;
    copyByte[0x27] = source[0x27];
    flag = D_1F800044;
    flag[0x2B] = flag[0xB] == 0;
    work = D_1F800044;
    if (work[0xB] != 0) {
      work[0] |= 0x20;
      mode = D_1F800044;
      mode[0x5C] = 1;
      tint = (s8 *)D_1F800044;
      tint[0x5F] = -0x40;
      tint[0x5E] = -0x40;
      tint[0x5D] = -0x40;
      work = D_1F800044;
    }
    *(u32 *)(work + 0x14) = 0x80;
    *(s32 *)(work + 0x20) = -2;
    *(u32 *)(work + 0x40) = 0x10000u;
    *(u32 *)(work + 0x44) = 0x10000u;
    work[0x48] = 2;
    setCountdown = D_1F800044;
    setCountdown[9] = 0x10;
    step = D_1F800044;
    step[2]++;
  }
}
