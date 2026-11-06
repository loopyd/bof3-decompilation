#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. The sibling bmagic lifts read the same
 * record: seedWorkWordsAndCopyRecordByte27ThenAdvanceWorkByte2BmagicMagic01500
 * _801F0D48 copies its byte 0x27 and the two exact motion steps of this overlay
 * copy its 0x34/0x38/0x3C word triplet. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler byte in main RAM, outside this overlay's image. The battle
 * targets own the address; this lift only reads it to separate a party battler
 * (below three) from an enemy, exactly as the battle lifts read it when they
 * select the 0x140-stride party record or the 0x118-stride enemy record. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before each store group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EFAE4
 * @behavior Entry 12 of the overlay handler table at 0x801EEC04, the word the
 *           table stores at 0x801EEC34. It first advances work byte 2, the
 *           handler index, by one when the acting battler byte 0x80146374 is a
 *           party member (below three) and by two when it is an enemy, keeping
 *           the new value in a byte-width add and storing it once after the
 *           branch. Both arms read the scratchpad work object through the
 *           pointer cell 0x1F800044 because the original re-materializes that
 *           cell load in each arm. It then refreshes the work object from the
 *           shared battle work record read through 0x801EB4E0: byte 0x27 is
 *           copied, byte 0x2B is set to 1, and words 0x34, 0x38 and 0x3C are
 *           copied. Each of those three store groups reads the pointer cell
 *           through its own local, because a store through the loaded pointer
 *           may alias the cell. Leaf, no frame, nothing returned; it takes no
 *           arguments and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2ByPartyOrEnemyStepThenCopyRecordByte27SeedByte2BAndCopyRecordTripletBmagicMagic01500_801EFAE4(
    void) {
  u8 *step;
  u8 next;
  u8 *source;
  u8 *copyByte;
  u8 *seed;
  u8 *record;
  u8 *work;

  if (D_80146374 < 3u) {
    step = D_1F800044;
    next = step[2] + 1;
  } else {
    step = D_1F800044;
    next = step[2] + 2;
  }
  step[2] = next;
  source = D_801EB4E0;
  copyByte = D_1F800044;
  copyByte[0x27] = source[0x27];
  seed = D_1F800044;
  seed[0x2B] = 1;
  record = D_801EB4E0;
  work = D_1F800044;
  *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
  *(u32 *)(work + 0x3C) = *(u32 *)(record + 0x3C);
}
