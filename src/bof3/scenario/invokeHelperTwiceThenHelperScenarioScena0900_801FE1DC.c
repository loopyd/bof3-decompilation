#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE1DC
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 4, then the same helper with the constant argument 8, and
 * finally the helper at 0x801C1630 with no arguments; it takes no arguments,
 * returns nothing and touches no state of its own. The 0x18-byte frame only
 * keeps $ra across the three calls. The address occupies the word stored at
 * 0x801FE53C, 0-based index 32 of the 38-entry in-image code-pointer list at
 * 0x801FE4BC that is terminated by a null word at 0x801FE554, so the overlay
 * invokes it indirectly and no in-image jal targets it. The 48 bytes are the
 * same three-call shape as the adjacent exact entry 31
 * invokeHelperTwiceThenHelperScenarioScena0900_801FE1AC (0x801FE1AC, arguments
 * 2 and 8), from which they differ only in the first constant argument
 * (4 instead of 2).
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperTwiceThenHelperScenarioScena0900_801FE1DC(void) {
  func_801C1ABC(4);
  func_801C1ABC(8);
  func_801C1630();
}
