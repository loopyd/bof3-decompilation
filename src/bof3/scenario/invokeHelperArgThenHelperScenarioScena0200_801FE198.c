#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE198
 * @behavior Thin overlay wrapper calling the helper at 0x801C1ABC with the
 * constant argument 4 and then the helper at 0x801C1630 with no arguments;
 * takes no arguments and returns nothing, and the 0x18-byte frame only keeps
 * $ra across the two calls. No in-image jal targets the address; its only image
 * reference is the word at 0x801FE374, entry 28 of the overlay callback table
 * D_801FE304 that func_801FD084 indexes with a record's byte field 0x7A. Its 40
 * bytes are byte-identical to the exact siblings
 * invokeHelperArgThenHelperScenarioScena0100_801FE244 (0x801FE244, scena01/00),
 * invokeHelperArgThenHelperScenarioScena0900_801FE12C (0x801FE12C, scena09/00),
 * invokeHelperArgThenHelperScenarioScena1100_801FADF8 (0x801FADF8, scena11/00)
 * and invokeHelperArgThenHelperScenarioScena1200_801FD0B0 (0x801FD0B0,
 * scena12/00), which share the same two-helper body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgThenHelperScenarioScena0200_801FE198(void) {
  func_801C1ABC(4);
  func_801C1630();
}
