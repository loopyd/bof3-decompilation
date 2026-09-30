#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE38C
 * @behavior Thin overlay dispatch handler: it calls the helper at 0x801C1ABC
 * with the constant argument 5, then the same helper with the constant argument
 * 6, and finally the helper at 0x801C1630 with no arguments. It takes no
 * arguments, returns nothing and touches no state of its own; the 0x18-byte
 * frame only keeps $ra across the three calls (0x801FE38C addiu $sp,-0x18 /
 * sw $ra,0x10($sp) / lw $ra,0x10($sp) / addiu $sp,0x18 / jr $ra, 0x30 bytes).
 * The address occupies the word stored at 0x801FE4E0, 0-based entry 21 and the
 * last non-null entry of the 22-entry in-image code-pointer list at
 * 0x801FE48C it shares with the reviewed sibling
 * invokeHelperTwiceThenHelperScenarioScena0600_801FE2B4, so the overlay
 * dispatches it indirectly and no in-image jal targets it; the list is
 * terminated by a null word at 0x801FE4E4. The 48 bytes are the same three-call
 * shape as the adjacent in-image entries 14
 * invokeHelperTwiceThenHelperScenarioScena0600_801FE254 (arguments 0 and 2), 15
 * invokeHelperTwiceThenHelperScenarioScena0600_801FE284 (arguments 0 and 5) and
 * 16 invokeHelperTwiceThenHelperScenarioScena0600_801FE2B4 (arguments 0 and 6),
 * and as the exact scena09 siblings
 * invokeHelperTwiceThenHelperScenarioScena0900_801FE154 and
 * invokeHelperTwiceThenHelperScenarioScena0900_801FE1AC in the other overlays
 * built from the same helper set; it differs from those only in the constant
 * arguments handed to 0x801C1ABC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperTwiceThenHelperScenarioScena0600_801FE38C(void) {
  func_801C1ABC(5);
  func_801C1ABC(6);
  func_801C1630();
}
