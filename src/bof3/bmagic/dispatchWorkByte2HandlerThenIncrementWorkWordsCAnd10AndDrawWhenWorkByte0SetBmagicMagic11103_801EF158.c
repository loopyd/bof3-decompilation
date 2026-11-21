#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and by every exact sibling of this handler in the same overlay family. It is
 * kept as a named cell rather than the constant-address memory accessor because
 * the original re-materialises the base at each access and reloads it after the
 * table call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Step pointers based at 0x801EFAA8, the word the scaled work byte 2 adds to.
 * That base is element 3 of the ten-pointer handler run this overlay keeps as
 * the textbin block at 0x801EFA9C, so the reference binds at the interior word
 * and leaves the reviewed data rows alone. Its members are 0x801EF1D4,
 * 0x801EF238, 0x801EF288, 0x801EF2D4, 0x801EF870, 0x801EF8C4 and 0x801EF914,
 * the last word of the run. */
extern void (*D_801EFAA8[])(void); /* @source 0x801EFAA8 @kind data */

/* Local draw routine: picks the draw-mode argument from GetGraphType, sets
 * draw mode, semi-transparency and one POLY_G4 packet and runs the matrix
 * helper at 0x801782FC twelve times. */
extern void func_801EF330(void); /* @source 0x801EF330 @kind function */

/* @source 0x801EF158
 * @behavior Entry 1 of the ten-pointer handler table at 0x801EFA9C. Reads the
 *           scratchpad work-object pointer cell at 0x1F800044 before frame
 *           creation and calls the step its unsigned work byte 2 selects from
 *           the table based at 0x801EFAA8, with no bounds test and no
 *           arguments. It then re-reads that cell, because the step call may
 *           repoint it, and only when work byte 0 is nonzero increments the
 *           object's words 0xC and 0x10 by one and calls the local draw
 *           routine at 0x801EF330, whose delay slot takes the second of those
 *           two stores. Takes no arguments, returns nothing, keeps a 0x18-byte
 *           frame that saves $ra and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenIncrementWorkWordsCAnd10AndDrawWhenWorkByte0SetBmagicMagic11103_801EF158(void) {
  D_801EFAA8[g_battle_work[2]]();
  if (g_battle_work[0] != 0) {
    *(s32 *)(g_battle_work + 0xC) += 1;
    *(s32 *)(g_battle_work + 0x10) += 1;
    func_801EF330();
  }
}
