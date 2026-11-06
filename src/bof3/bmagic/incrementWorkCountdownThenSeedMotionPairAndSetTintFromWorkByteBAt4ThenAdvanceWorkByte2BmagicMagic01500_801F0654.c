#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a target-local named symbol so
 * every access re-materializes the cell address as a lui plus offset load, which
 * is how the original reads it before each store group. The exact siblings of
 * this target bind it the same way:
 * decrementWorkCountdownThenSeedMotionFieldsAndSetTintWhenGlobalWorkByteBIsSet
 * BmagicMagic01500_801F045C and
 * advanceWorkByte2ByPartyOrEnemyStepThenCopyRecordByte27SeedByte2BAnd
 * CopyRecordTripletBmagicMagic01500_801EFAE4. The D_ name auto-binds the
 * address through the composed Splat maps, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F0654
 * @behavior Entry 27 of this overlay's header handler table, the code-pointer
 *           run that starts at 0x801EEC04 and stores this function's address at
 *           0x801EEC70. It reads the scratchpad work-object pointer cell at
 *           0x1F800044 once, increments the object's countdown byte 9 and
 *           stores it back with sb, keeping the incremented value in a byte
 *           local so the truncation, not a reload, supplies the compared value.
 *           While the new countdown is not four it only returns, which is the
 *           four-frame wait the decrementing siblings spend before their own
 *           threshold. When the countdown reaches four it re-reads the pointer
 *           cell, reads the object's byte 0xB into a value local, and seeds the
 *           16.16 motion pair at 0x40 and 0x44 with 0x8000 and 0x18000 before
 *           branching on that byte. When byte 0xB is nonzero it writes the
 *           signed tint triplet from it: channel 0x5D takes -(byte 0xB * 16 +
 *           0x30) computed from the saved local, while channels 0x5E and 0x5F
 *           each re-read the pointer cell and byte 0xB and take -(byte 0xB * 16
 *           + 0x40), so the two of them repeat the multiply rather than sharing
 *           one product. The 0x5D store reuses the work pointer the motion seed
 *           left live, which is why only channels 0x5E and 0x5F materialize the
 *           cell again. When byte 0xB is zero it raises bit 0x20 of work byte 0
 *           through that live pointer, sets mode byte 0x5C to 1, and sets the
 *           signed tint triplet to 0x30, -0x40 and -0x40 (0xD0, 0xC0, 0xC0
 *           unsigned), with the 0x5E and 0x5F pair sharing one materialized
 *           -0x40 across two separate pointer-cell reads. Both arms then join
 *           and advance work byte 2, the handler index, with a byte-width
 *           read-modify-write. Each tint store reads the cell through its own
 *           expression, because a store through the loaded pointer may alias
 *           the cell: spelling the channels through one pointer local instead
 *           measures 60/71 and four bytes short, since the resulting single
 *           pseudo is allocated over the constant and the two arms then end in
 *           identical `sb $v0, 0x5F($v1)` instructions that the compiler
 *           cross-jumps into one store. Leaf, no frame, nothing returned; it
 *           takes no arguments and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkCountdownThenSeedMotionPairAndSetTintFromWorkByteBAt4ThenAdvanceWorkByte2BmagicMagic01500_801F0654(void) {
  u8 *work;
  u8 countdown;
  u8 *object;
  u32 channel;
  u8 *step;

  work = D_1F800044;
  countdown = work[9] + 1;
  work[9] = countdown;
  if (countdown == 4) {
    object = D_1F800044;
    channel = object[0xB];
    *(u32 *)(object + 0x40) = 0x8000u;
    *(u32 *)(object + 0x44) = 0x18000u;
    if (channel != 0) {
      ((s8 *)D_1F800044)[0x5D] = -(channel * 16 + 0x30);
      ((s8 *)D_1F800044)[0x5E] = -(((u8 *)D_1F800044)[0xB] * 16 + 0x40);
      ((s8 *)D_1F800044)[0x5F] = -(((u8 *)D_1F800044)[0xB] * 16 + 0x40);
    } else {
      object[0] |= 0x20;
      ((s8 *)D_1F800044)[0x5C] = 1;
      ((s8 *)D_1F800044)[0x5D] = 0x30;
      ((s8 *)D_1F800044)[0x5E] = -0x40;
      ((s8 *)D_1F800044)[0x5F] = -0x40;
    }
    step = D_1F800044;
    step[2]++;
  }
}
