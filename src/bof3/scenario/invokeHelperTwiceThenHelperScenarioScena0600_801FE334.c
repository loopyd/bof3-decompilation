#include "bof3/bof3.h"

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE334
 * @behavior Thin overlay dispatch handler: it calls the helper at 0x801C1ABC
 * with the constant argument 6, then the same helper with the constant argument
 * 2, and finally the helper at 0x801C1630 with no arguments. It takes no
 * arguments, returns nothing and touches no state of its own; the 0x18-byte
 * frame only keeps $ra across the three calls (0x801FE334 addiu $sp,-0x18 /
 * sw $ra,0x10($sp) / lw $ra,0x10($sp) / addiu $sp,0x18 / jr $ra / nop, 0x30
 * bytes). The address occupies the word stored at 0x801FE4D8, 0-based entry 19
 * of the 22-entry in-image code-pointer list at 0x801FE48C, which is
 * terminated by a null word at 0x801FE4E4; no in-image jal targets it and that
 * word is the only mention of the address in the payload.
 * dispatchCoordinateLookupHandlerScenarioScena0600_801FDD14 indexes the list
 * with the answer of func_801A7AD8, whose four five-byte lookup records at
 * 0x801FE478 reach entries 0 through 3 only. The 48 bytes are the same
 * three-call shape as the adjacent in-image entry 21
 * invokeHelperTwiceThenHelperScenarioScena0600_801FE38C (arguments 5 and 6),
 * which was lifted exact, and as entries 14
 * invokeHelperTwiceThenHelperScenarioScena0600_801FE254 (arguments 0 and 2), 15
 * invokeHelperTwiceThenHelperScenarioScena0600_801FE284 (arguments 0 and 5) and
 * 16 invokeHelperTwiceThenHelperScenarioScena0600_801FE2B4 (arguments 0 and 6);
 * it differs from those bodies only in the two constant arguments handed to
 * 0x801C1ABC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperTwiceThenHelperScenarioScena0600_801FE334(void) {
  func_801C1ABC(6);
  func_801C1ABC(2);
  func_801C1630();
}
