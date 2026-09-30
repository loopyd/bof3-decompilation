#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FAE70
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 5 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing. The 0x18-byte frame only keeps $ra
 * across the two calls. The address is word 46 (0-based 45) of the contiguous
 * code-pointer run 0x801FAEDC-0x801FAF94 that ends the payload, so the overlay
 * dispatches this entry indirectly and no in-image jal targets it. It is the
 * last of the byte-identical three-wrapper family at 0x801FAE20, 0x801FAE48
 * and 0x801FAE70, which differ only in the constant argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena1100_801FAE70(void) {
  func_801C1ABC(5);
  func_801C1630();
}
