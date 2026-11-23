#include "bof3/bof3.h"

/* Shared overlay state byte of the main executable, outside this payload: the
 * trail reset at 0x801F2E14 clears it, the entry hook at 0x801F2DE8 arms it to
 * 1 and the step handler at 0x801F2C50 arms it to 4. This body reads it once
 * and takes it as the unchecked index of the overlay handler run. */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* Handler run of this overlay: the first seven words of the pointer data the
 * reviewed Splat layout places at payload offset 0x18B8 are the overlay step
 * handlers 0x801F2C50, 0x801F2C7C, 0x801F2CC0, 0x801F2CE0, 0x801F2D7C,
 * 0x801F2DE8 and 0x801F2E14, and the word at 0x801F44D4 that follows them is
 * zero. */
extern void (*D_801F44B8[])(void); /* @source 0x801F44B8 @kind data */

/* The two-layer trail draw this overlay entry runs after the selected step. */
extern void
renderPairsAdvancedClock(
    void);

/* Textured-quad draw routine of this overlay: it issues SetDrawMode,
 * SetPolyFT4, SetShadeTex and SetSemiTrans packets over its three passes. */
/* @source 0x801F400C @kind function */
extern void func_801F400C(void);

/* @source 0x801F2C04
 * @behavior Per-frame entry hook of this overlay, the code span directly behind
 *           its four-byte payload header; no jal and no word of the payload
 *           names it, so the overlay loader is its only caller and its argument
 *           roles stay structural, of which it uses none. It reads the shared
 *           overlay state byte at 0x8014933B once with a `lbu`, scales it by
 *           four and calls the word at that index of this overlay's handler run
 *           at 0x801F44B8 without bounding the index first. The first seven
 *           entries of that run are the overlay step handlers 0x801F2C50,
 *           0x801F2C7C, 0x801F2CC0, 0x801F2CE0, 0x801F2D7C, 0x801F2DE8 and
 *           0x801F2E14, and its eighth word is zero. After the indirect call it
 *           calls the two-layer trail draw at 0x801F3398 and then the
 *           textured-quad draw routine at 0x801F400C, discards the handler's
 *           result and reads no other state, writing none. The 0x18-byte frame
 *           only keeps $ra at 0x10 across the three calls; the state byte load
 *           is scheduled before the frame creation, the slot after the table
 *           load stays a nop, and every call returns into a nop delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callOverlayLayerTrail(
    void) {
  D_801F44B8[D_8014933B]();
  renderPairsAdvancedClock();
  func_801F400C();
}
