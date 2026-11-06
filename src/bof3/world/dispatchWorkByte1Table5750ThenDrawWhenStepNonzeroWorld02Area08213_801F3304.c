#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, read once per access: the original
 * re-materialises its address (lui $vN,0x1F80 / lw $vN,68($vN)) for each of the
 * two reads, so the cell is named here instead of reached through
 * SPAD_PTR_SLOT, whose constant address GCC folds into one shared base. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Five-entry overlay step-handler table for work-object byte 1, entries
 * 0x801F336C through 0x801F34B8. */
extern void (*D_801F5750[])(void); /* @source 0x801F5750 @kind data */

/* Draws the sixteen quad descriptors this overlay advances each frame. */
extern void func_801F3500(void);

/* @source 0x801F3304
 * @behavior Overlay step dispatcher: reads the scratchpad work-object pointer
 *           cell at 0x1F800044 and uses the work-object byte at offset 0x1,
 *           scaled by four, to select one entry of the five-entry handler table
 *           at 0x801F5750, which it calls with no arguments. When the handler
 *           returns it reads the pointer cell again in a second load; only when
 *           the work-object byte at offset 0x1 is still non-zero does it call
 *           the quad-drawing helper at 0x801F3500, so a handler that advanced
 *           the phase past its last entry skips the drawing step. Takes no
 *           arguments, stores nothing and returns nothing; the handler roles,
 *           the table's five entries and the drawing helper's role stay
 *           anchored to their addresses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Table5750ThenDrawWhenStepNonzeroWorld02Area08213_801F3304(void) {
  D_801F5750[D_1F800044[1]]();
  if (D_1F800044[1] != 0) {
    func_801F3500();
  }
}
