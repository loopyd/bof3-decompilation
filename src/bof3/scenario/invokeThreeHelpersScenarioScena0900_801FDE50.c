#include "bof3/bof3.h"

void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FDE50
 * @behavior Thin overlay handler: it calls the helper at 0x801C1400 with the
 * constant argument 0, then the helper at 0x801C187C with the constant argument
 * 6, and finally the helper at 0x801C1630 with no arguments. It returns nothing,
 * reads no memory of its own and writes none; the 0x18-byte frame only keeps $ra
 * across the three calls. The address occupies the word stored at 0x801FE508,
 * 0-based index 19 of the 38-entry in-image code-pointer handler table at
 * 0x801FE4BC that is terminated by a null word at 0x801FE554, so the overlay
 * invokes it indirectly through func_801FD280 and no in-image jal targets it.
 * The two entries around it (0x801FDE0C and 0x801FDE80) run the same three-call
 * tail preceded by func_80166E88(7, ...).
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeThreeHelpersScenarioScena0900_801FDE50(void) {
  func_801C1400(0);
  func_801C187C(6);
  func_801C1630();
}
