#include "bof3/bof3.h"

extern u8 D_8014933A;
extern u8 g_ScenarioProgress;

/* Screen-gradient fill helper, the overlay's own routine at 0x801F5428. Its
 * body splits the packed word it is handed into two five-bit-per-channel
 * colours (top edge from bits 0-14, bottom edge from bits 16-30, each scaled
 * left by three), draws a 0x140 x 0xF0 SetPolyG4 quad through the primitive
 * cursor cell at 0x8014598C with one colour on the top edge and the other on
 * the bottom edge, advances that cursor by 0x24 and returns nothing. Splat
 * binds the address through this target's map row. */
void func_801F5428(s32 color);

/* @source 0x801F527C
 * @behavior Overlay step handler, entry index 14 of the sixteen-pointer step
 *           table at 0x801F57D0. It clears the shared palette-stage byte at
 *           0x8014933A, then hands the constant colour word 0x10001805 to the
 *           screen-gradient fill helper at 0x801F5428, which paints the whole
 *           0x140 x 0xF0 screen. It then reads the shared scenario progress
 *           byte at 0x80146864 and, only when that byte is 0x3B, re-reads the
 *           scratchpad work pointer cell 0x1F800044 and advances that work
 *           record's dispatch byte at offset 0x02 by one, which passes control
 *           to the next handler; every other progress value leaves the record
 *           untouched. The function has a 0x18-byte frame holding the saved
 *           return address at 0x10 and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fillScreenGradientAdvanceWorkByte2AtScenarioProgress3bWorld02Area08213_801F527C(
    void) {
  u8* work;

  D_8014933A = 0;
  func_801F5428(0x10001805);
  if (g_ScenarioProgress == 0x3B) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[2]++;
  }
}
