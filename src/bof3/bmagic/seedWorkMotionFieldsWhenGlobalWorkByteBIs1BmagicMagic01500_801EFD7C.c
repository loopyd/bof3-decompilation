#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. The sibling bmagic lifts read the same
 * record: seedWorkWordsAndCopyRecordByte27ThenAdvanceWorkByte2BmagicMagic01500
 * _801F0D48 copies its word triplet and byte 0x27, and
 * incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4 guards on
 * the same byte 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before each store group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EFD7C
 * @behavior Entry 0x11 of the overlay handler table at 0x801EEC04: only when
 *           byte 0xB of the shared battle work record read through 0x801EB4E0
 *           equals 1 does it seed the scratchpad work object published at the
 *           pointer cell 0x1F800044. The seed sets both words of the 16.16
 *           motion pair at offsets 0x40 and 0x44 to 0x10000, byte 0x48 to 2,
 *           byte 9 (the countdown seeded with 0x1E by the 0x801F0330 shape) to
 *           8, and then advances work byte 2, the handler index, with a
 *           byte-width read-modify-write. Each store group reads the pointer
 *           cell through its own local because the original reloads the cell
 *           after stores through the loaded pointer. The guard runs before the
 *           cell is touched, so 0x1F800044 is only dereferenced on the taken
 *           path; leaf, no frame, nothing returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkMotionFieldsWhenGlobalWorkByteBIs1BmagicMagic01500_801EFD7C(void) {
  u8 *work;
  u8 *setCountdown;
  u8 *step;

  if (D_801EB4E0[0xB] == 1) {
    work = D_1F800044;
    *(u32 *)(work + 0x40) = 0x10000u;
    *(u32 *)(work + 0x44) = 0x10000u;
    work[0x48] = 2;
    setCountdown = D_1F800044;
    setCountdown[9] = 8;
    step = D_1F800044;
    step[2]++;
  }
}
