#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The record's state byte sits at +2 and
 * the overlay dispatcher at 0x801F2C04 indexes this overlay's handler table with
 * it, so the increment below is what selects the next handler.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared main-executable byte this handler gates on; the sibling scenario lifts
 * declare the same word and this overlay's own map binds no name for it.
 * @source 0x8014933B @kind data
 */
extern u8 D_8014933B;

/* This overlay's cell-quad draw. Its argument is a per-call displacement that
 * shifts every cell edge and the third quad's slip, so the zero passed here
 * draws the cells unshifted.
 */
extern void renderTexturedQuadsShifted(
    s32 shift);

/* @source 0x801F2C94
 * @behavior State-1 handler of this overlay's work-byte-2 dispatch: the payload's
 *           handler table at 0x801F3320 holds 0x801F2C48, 0x801F2C94, 0x801F2CDC
 *           and 0x801F2D38 in that order, and the dispatcher at 0x801F2C04 calls
 *           the word its index selects. While the shared main-executable byte
 *           0x8014933B reads anything but 1 the handler writes nothing and
 *           returns. Once it reads 1 the handler advances the work record's
 *           unsigned state byte at +2, reached through the scratchpad pointer
 *           cell 0x1F800044, so the next dispatch selects the state-2 handler at
 *           0x801F2CDC, and calls this overlay's cell-quad draw with a zero
 *           displacement in the same step, so the three records at 0x801F3330
 *           are drawn with no edge or depth shift. Takes no arguments, returns
 *           nothing, and the 0x18-byte frame only keeps the return address at
 *           0x10 across that call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2AndDrawThreeCellQuadsWhenSharedByte1World02Area10513_801F2C94(
    void) {
  if (D_8014933B == 1) {
    D_1F800044[2]++;
    renderTexturedQuadsShifted(0);
  }
}
