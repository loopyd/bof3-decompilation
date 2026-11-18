#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE30C
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 5 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing. The 0x18-byte frame only keeps $ra
 * across the two calls. The address occupies the word stored at 0x801FE548,
 * 0-based index 35 of the 38-entry in-image code-pointer list at 0x801FE4BC
 * that is terminated by a null word at 0x801FE554, so the overlay invokes it
 * indirectly and no in-image jal targets it. A comparison of the two payload
 * ranges measured these 40 bytes equal to
 * invokeHelperArgThenHelperScenarioScena0900_801FE334 (argument 6) and to
 * invokeHelperArgThenHelperScenarioScena0900_801FE20C (argument 8) at every
 * byte except payload offset 12, the argument immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena0900_801FE30C(void) {
  func_801C1ABC(5);
  func_801C1630();
}
