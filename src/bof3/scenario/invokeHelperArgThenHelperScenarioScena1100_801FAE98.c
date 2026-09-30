#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FAE98
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 8 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing. The 0x18-byte frame only keeps $ra
 * across the two calls. The address is the target of the last word (0x801FAF94)
 * of the contiguous code-pointer run 0x801FAEDC-0x801FAF98 that ends the
 * payload, so the overlay dispatches this entry indirectly and no in-image jal
 * targets it. The 40 bytes are byte-identical to the exact siblings
 * invokeHelperArgThenHelperScenarioScena1100_801FADF8 (0x801FADF8),
 * invokeHelperArgThenHelperScenarioScena1100_801FAE20 (0x801FAE20),
 * invokeHelperArgThenHelperScenarioScena1100_801FAE48 (0x801FAE48) and
 * invokeHelperArgThenHelperScenarioScena1100_801FAE70 (0x801FAE70), which
 * differ only in the constant argument (4, 2, 6 and 5 respectively), and to the
 * cross-overlay copies of the same body in scena09/00 and scena12/00.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena1100_801FAE98(void) {
  func_801C1ABC(8);
  func_801C1630();
}
