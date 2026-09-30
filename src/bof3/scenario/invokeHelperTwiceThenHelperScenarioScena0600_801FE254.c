#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE254
 * @behavior Thin overlay dispatch handler: it calls the helper at 0x801C1ABC
 * with the constant argument 0, then the same helper with the constant argument
 * 2, and finally the helper at 0x801C1630 with no arguments. It takes no
 * arguments, returns nothing and touches no state of its own; the 0x18-byte
 * frame only keeps $ra across the three calls. The address occupies the word
 * stored at 0x801FE4C4, 0-based entry 14 of the 22-entry in-image code-pointer
 * list at 0x801FE48C (0x801FDD7C ... 0x801FE38C) that the overlay dispatcher
 * func_801FDD14 indexes with the signed byte returned by
 * func_801A7AD8(0x801FE478, 4) and then calls with no arguments, so the overlay
 * invokes it indirectly and no in-image jal targets it; the list is terminated
 * by a null word at 0x801FE4E4. The 48 bytes are the same three-call shape as
 * the adjacent in-image entries 15
 * invokeHelperTwiceThenHelperScenarioScena0600_801FE284 (arguments 0 and 5) and
 * 16 func_801FE2B4 (arguments 0 and 6), and as the exact scena09 siblings
 * invokeHelperTwiceThenHelperScenarioScena0900_801FE154 and
 * invokeHelperTwiceThenHelperScenarioScena0900_801FE1AC in the other overlays
 * built from the same helper set; it differs from those only in the constant
 * arguments handed to 0x801C1ABC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperTwiceThenHelperScenarioScena0600_801FE254(void) {
  func_801C1ABC(0);
  func_801C1ABC(2);
  func_801C1630();
}
