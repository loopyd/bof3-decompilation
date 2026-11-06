#include "bof3/bof3.h"

/* Shared per-frame counter in main RAM; its low bit selects the sign of the
 * ramp this handler publishes.
 * @source 0x80143E6C @kind data
 */
extern s32 D_80143E6C;

/* Shared status byte that receives the mode constant 2.
 * @source 0x80149333 @kind data
 */
extern u8 D_80149333;

/* Shared halfword counter that receives the signed ramp.
 * @source 0x8014932A @kind data
 */
extern u16 D_8014932A;

/* Scratchpad work-object pointer cell. The original re-materialises the
 * constant per access (lui 0x1F80 plus lw 0x44), so the cell is named: a bare
 * scratchpad constant folds into one CSE'd base plus lw 0, which diverges.
 * @source 0x1F800044 @kind data
 */
extern u8 *g_battle_work;

/* @source 0x801EF218
 * @behavior Step handler 1 of this overlay's four-word step run at 0x801EEC04,
 * whose words are 0x801EEC80, 0x801EF218, 0x801EF2B8 and 0x801EFD7C: the
 * per-frame dispatcher func_801EEC14 copies those words into its frame and
 * calls the one selected by unsigned work byte 0x01, so work byte 0x01 holds 1
 * while this body runs and no jal in the image targets the address. It advances
 * the scratchpad work object's byte 0x09 by one through a single pointer-cell
 * read of 0x1F800044 (lbu / addiu 1 / sb) and, when the truncated count reaches
 * 0x10, re-reads the cell to clear byte 0x09 and re-reads it once more to
 * increment work byte 0x01, stepping the dispatcher to handler 2 after sixteen
 * frames. It then publishes mode 2 in the shared status byte D_80149333 and
 * stores the refreshed work byte 0x09 halved and scaled by a sign into the
 * shared halfword counter D_8014932A, that sign being +1 or -1 according to bit
 * 0 of the shared per-frame counter D_80143E6C. It reads only D_80143E6C and
 * the work object, writes the work object's bytes, D_80149333 and D_8014932A,
 * and is a leaf with no frame and no saved register: it takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte1Every16FramesAndPublishSignedRampToHalfword8014932ABmagicMagic10203_801EF218(
    void) {
  u8 *work;
  u8 count;
  s32 step;

  work = g_battle_work;
  count = (u8)(work[9] + 1);
  work[9] = count;
  if (count >= 0x10) {
    g_battle_work[9] = 0;
    g_battle_work[1] += 1;
  }
  step = (D_80143E6C & 1) * 2 - 1;
  D_8014932A = (u16)(step * (g_battle_work[9] >> 1));
  D_80149333 = 2;
}
