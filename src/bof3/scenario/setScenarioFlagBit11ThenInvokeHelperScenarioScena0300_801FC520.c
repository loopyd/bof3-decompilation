#include "bof3/bof3.h"

void func_8015B580(u32 arg0, s32 arg1);
void func_801FC5A4(void);

/* @source 0x801FC520
 * @behavior Handler slot 2 of the overlay's nine-pointer handler table at
 * 0x801FD00C (D_801FD00C). The frame dispatcher at 0x801FC46C selects a slot
 * from a record byte and calls the handler as (record, D_8014686C), so this
 * handler sets flag bit 0x11 of the scenario flag word arriving in $a1 through
 * the shared bit-set helper at 0x8015B580 and then invokes the no-argument
 * helper at 0x801FC5A4; the record pointer in $a0 is overwritten by that flag
 * word and never read, and both helper results are discarded. It takes two
 * arguments and returns nothing. Instruction for instruction it is the slot 1
 * twin setScenarioFlagBit10ThenInvokeHelperScenarioScena0300_801FC4F4 with only
 * the bit constant (0x11 instead of 0x10) in the delay slot of the first call
 * differing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit11ThenInvokeHelperScenarioScena0300_801FC520(s32 arg0, s32 arg1) {
  func_8015B580(arg1, 0x11);
  func_801FC5A4();
}
