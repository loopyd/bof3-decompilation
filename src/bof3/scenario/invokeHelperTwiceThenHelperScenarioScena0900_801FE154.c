#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE154
 * @behavior Thin overlay handler: it calls the helper at 0x801C1ABC with the
 * constant argument 4, then the same helper with the constant argument 2, and
 * finally the helper at 0x801C1630 with no arguments. It returns nothing, reads
 * no memory of its own and writes none; the 0x18-byte frame only keeps $ra
 * across the three calls. The address occupies the word stored at 0x801FE530,
 * 0-based index 29 of the 38-entry in-image code-pointer handler table at
 * 0x801FE4BC that is terminated by a null word at 0x801FE554, so the overlay
 * invokes it indirectly through func_801FD280 and no in-image jal targets it.
 * The 48 bytes are the same three-call shape as the adjacent exact entry 28
 * invokeHelperArgThenHelperScenarioScena0900_801FE12C (0x801FE12C), which calls
 * 0x801C1ABC with 4 and then 0x801C1630, and as the exact scena15 sibling
 * invokeHelperTwiceThenHelperScenarioScena1500_801FE49C (0x801FE49C); it
 * differs from the first only by the extra 0x801C1ABC(2) call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperTwiceThenHelperScenarioScena0900_801FE154(void) {
  func_801C1ABC(4);
  func_801C1ABC(2);
  func_801C1630();
}
