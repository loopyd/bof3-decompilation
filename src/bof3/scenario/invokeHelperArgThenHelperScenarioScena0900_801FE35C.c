#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE35C
 * @behavior Entry 37 (0-based), the last entry, of the overlay's 38-entry
 * in-image code-pointer list at 0x801FE4BC that is terminated by a null word
 * at 0x801FE554: a thin wrapper that calls the helper at 0x801C1ABC with the
 * constant argument 7 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing. The 0x18-byte frame only keeps $ra
 * across the two calls. No in-image jal targets it, so the overlay invokes it
 * indirectly through that table. The 40 bytes are byte-identical to the exact
 * sibling invokeHelperArgThenHelperScenarioScena0900_801FE334 (0x801FE334,
 * argument 6) and, transitively, to
 * invokeHelperArgThenHelperScenarioScena0900_801FE12C (0x801FE12C, argument 4),
 * invokeHelperArgThenHelperScenarioScena0900_801FE184 (0x801FE184, argument 2)
 * and invokeHelperArgThenHelperScenarioScena0900_801FE20C (0x801FE20C,
 * argument 8), which differ only in the constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena0900_801FE35C(void) {
  func_801C1ABC(7);
  func_801C1630();
}
