#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The record's byte at +9 is the
 * displacement this overlay's cell-quad draw receives, and this handler is the
 * only site in the payload that advances it, so the cells and the third quad's
 * slip step once per call. The pointer is re-read after the reset call because
 * that call may replace the record.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared main-executable overlay state byte. The lifted siblings of this
 * target declare the same word and this overlay's own map binds no name for it;
 * the world and scenario lifts publish 1, 2, 3 and 0x23 into it, and the state-1
 * handler of this same dispatch tests it for 1.
 * @source 0x8014933B @kind data
 */
extern u8 D_8014933B;

/* Shared main-executable reset helper of the main executable, outside this
 * payload: it clears the scratchpad game work-area header fields, and the
 * lifted sibling wrappers of the scenario overlays call it the same way. No jal
 * immediate targets it from inside this payload apart from this handler.
 * @source 0x80196070
 */
extern void func_80196070(void);

/* This overlay's cell-quad draw. Its argument is a per-call displacement that
 * shifts every cell edge and the third quad's slip.
 */
extern void drawThreeCellTexturedQuadsShiftedByArgWorld02Area10513_801F2D9C(
    s32 shift);

/* @source 0x801F2D38
 * @behavior State-3 handler of this overlay's work-byte-2 dispatch: the
 *           payload's handler table at 0x801F3320 holds 0x801F2C48, 0x801F2C94,
 *           0x801F2CDC and this address in that order, and the dispatcher at
 *           0x801F2C04 calls the word its index selects. The handler first
 *           advances the work record's unsigned counter byte at +9, reached
 *           through the scratchpad pointer cell 0x1F800044. While the shared
 *           main-executable overlay state byte 0x8014933B reads zero it then
 *           calls the shared work-area reset helper at 0x80196070, so the
 *           scratchpad work-area header is cleared on every step taken before
 *           the state byte is published. Either way it closes by re-reading the
 *           pointer cell, because the reset call may have replaced the record,
 *           and calls this overlay's three-cell-quad draw with the counter byte
 *           as the displacement, so the cells and the third quad's slip advance
 *           once per call. Takes no arguments, returns nothing, and the
 *           0x18-byte frame only keeps the return address at 0x10 across the
 *           reset and draw calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte9ResetWorkAreaWhenSharedStateByteClearThenDrawThreeCellQuadsWorld02Area10513_801F2D38(
    void) {
  D_1F800044[9]++;
  if (D_8014933B == 0) {
    func_80196070();
  }
  drawThreeCellTexturedQuadsShiftedByArgWorld02Area10513_801F2D9C(
      D_1F800044[9]);
}
