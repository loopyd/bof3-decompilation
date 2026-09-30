#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE184
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 2 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing. The 0x18-byte frame only keeps $ra
 * across the two calls. The address occupies the word stored at 0x801FE534,
 * 0-based index 30 of the 38-entry in-image code-pointer list at 0x801FE4BC
 * that is terminated by a null word at 0x801FE554, so the overlay invokes it
 * indirectly and no in-image jal targets it. The 40 bytes are byte-identical to
 * the exact sibling
 * invokeHelperArgThenHelperScenarioScena0900_801FE12C (0x801FE12C, argument 4)
 * and differ from it only in the constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena0900_801FE184(void) {
  func_801C1ABC(2);
  func_801C1630();
}
