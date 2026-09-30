#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE364
 * @behavior Entry 20 of this overlay's 22-pointer code-pointer list at
 * 0x801FE48C (0x801FDD7C ... 0x801FE38C), the list the overlay dispatcher
 * func_801FDD14 indexes with the signed byte returned by
 * func_801A7AD8(0x801FE478, 4) and then calls with no arguments: a thin wrapper
 * that calls the helper at 0x801C1ABC with the constant argument 5 and then the
 * helper at 0x801C1630 with no arguments; takes no arguments and returns
 * nothing. The 0x18-byte frame only keeps $ra across the two calls. Its 40 bytes
 * are byte-identical to the exact siblings
 * invokeHelperArgThenHelperScenarioScena0600_801FE2E4 (constant argument 2) and
 * invokeHelperArgThenHelperScenarioScena0600_801FE30C (constant argument 6),
 * which differ only in the constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena0600_801FE364(void) {
  func_801C1ABC(5);
  func_801C1630();
}
