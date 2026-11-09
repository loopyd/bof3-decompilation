#include "bof3/bof3.h"

extern u8 *D_1F800044;
void func_801F7FFC(u8 *work);
void drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170(
    u8 *work);
void func_80196070(void);

/* @source 0x801F7D80
 * @behavior Calls the local helpers 0x801F7FFC and 0x801F8170 with the captured
 * work pointer plus 0x0C, then subtracts 8 from that work object's halfword at
 * 0x58. When the stored halfword reads as a negative signed value the arm
 * reloads the scratchpad work pointer cell 0x1F800044, because the halfword
 * store can republish it, writes 1 to the object's transition byte at 0x06 and
 * resets the work state through the shared func_80196070 helper, which zeroes
 * work-object bytes 0x00-0x04 including the dispatch byte 1. Takes no arguments
 * and returns no value. The halfword's wider role is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 28/28 instructions, 112 -> 112 bytes on the first seed. The signed
 * test is spelled on the value the halfword store already wrote
 * (`halfword = *(u16 *)(work + 0x58) - 8;` then `if ((s16)halfword < 0)`), which
 * keeps the load, the subtract, the store and the `sll`/`bgez` test in one `$v0`
 * as the original does. The arm reads the pointer cell itself so the reload after
 * the halfword store survives, and the constant 1 lands in the branch delay slot.
 */
void subtract8FromWorkHalfword58AndResetWorkStateWhenNegativeScenarioScena0300_801F7D80(void) {
  u8 *work;
  u8 *helper_work;
  u32 halfword;

  work = D_1F800044;
  helper_work = work + 0x0C;
  func_801F7FFC(helper_work);
  drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170(
      helper_work);
  halfword = *(u16 *)(work + 0x58) - 8;
  *(u16 *)(work + 0x58) = halfword;
  if ((s16)halfword < 0) {
    D_1F800044[6] = 1;
    func_80196070();
  }
}
