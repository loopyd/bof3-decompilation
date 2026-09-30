#include "bof3/bof3.h"

void func_8015B580(u32 arg0, s32 arg1);
void func_801FC5A4(void);

/* @source 0x801FC4F4
 * @behavior Handler slot 1 of the overlay's nine-pointer handler table at
 * 0x801FD00C (D_801FD00C). The frame dispatcher at 0x801FC46C selects a slot
 * from a record byte and calls the handler as (record, D_8014686C), so this
 * handler sets flag bit 0x10 of the scenario flag word arriving in $a1 through
 * the shared bit-set helper at 0x8015B580 and then invokes the no-argument
 * helper at 0x801FC5A4; the record pointer in $a0 is overwritten by that flag
 * word and never read, and both helper results are discarded. It takes two
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit10ThenInvokeHelperScenarioScena0300_801FC4F4(s32 arg0, s32 arg1) {
  func_8015B580(arg1, 0x10);
  func_801FC5A4();
}
