#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE244
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 4 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing. The 0x18-byte frame only keeps $ra
 * across the two calls. The address is entry 5, the word at 0x801FE370, of the
 * overlay pointer table at 0x801FE35C, so the overlay invokes it indirectly and
 * no in-image jal targets it. The 40 bytes are byte-identical to the exact
 * siblings invokeHelperArgThenHelperScenarioScena0900_801FE12C (0x801FE12C,
 * scena09/00), invokeHelperArgThenHelperScenarioScena1100_801FADF8 (0x801FADF8,
 * scena11/00) and invokeHelperArgThenHelperScenarioScena1200_801FD0B0
 * (0x801FD0B0, scena12/00), which share the same two-helper body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena0100_801FE244(void) {
  func_801C1ABC(4);
  func_801C1630();
}
