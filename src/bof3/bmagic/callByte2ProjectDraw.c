#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*tableHandlerByte2C[])(void); /* @source 0x801F0384 @kind data */
extern void applyWorkProjectedTransform(void); /* @source 0x801EF1C8 @kind function */
extern void renderTranslucentTriangle(void); /* @source 0x801F01A8 @kind function */

/* @source 0x801F0038
 * @behavior Dispatch the handler indexed by work byte 2 through the table at
 *           0x801F0384, then re-read the scratchpad work pointer. When work
 *           bytes 0 and 2 are both nonzero, invoke the local transform helper
 *           at 0x801EF1C8, the draw helper at 0x801F01A8 and PopMatrix, in that
 *           order. The transform helper pushes the matrix stack and installs
 *           the work record's projected transform; the draw helper emits a
 *           translucent triangle. The dispatch index is unsigned and unbounded.
 *           Takes no arguments and returns nothing. The pointer at 0x801F035C
 *           references this entry; no jal in this payload names it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callByte2ProjectDraw(
    void) {
  tableHandlerByte2C[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    applyWorkProjectedTransform();
    renderTranslucentTriangle();
    PopMatrix();
  }
}
