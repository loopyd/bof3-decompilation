#include "bof3/bof3.h"

void func_8015B580(u32 arg0, s32 arg1);
void func_801FC5A4(void);

/* @source 0x801FC54C
 * @behavior Handler slot 3 of the overlay's nine-pointer handler table at
 * 0x801FD00C (D_801FD00C), whose slots 0x801FD00C..0x801FD02C are
 * {0x801FC4AC, 0x801FC4F4, 0x801FC520, 0x801FC54C, 0x801FC578, 0x801FC5FC,
 * 0x801FC688, 0x801FC690, 0x801FC6C4} and whose index is byte 0x7A of the
 * record dispatched by 0x801FC46C, which calls the selected slot as (record,
 * D_8014686C). This handler therefore sets flag bit 0x12 of the scenario flag
 * word arriving in $a1 through the shared bit-set helper at 0x8015B580 and
 * then invokes the no-argument helper at 0x801FC5A4; the record pointer in $a0
 * is overwritten by that flag word and never read, and both helper results are
 * discarded. It takes two arguments and returns nothing. Instruction for
 * instruction it is the slot 1 twin
 * setScenarioFlagBit10ThenInvokeHelperScenarioScena0300_801FC4F4 with only the
 * bit constant (0x12 instead of 0x10) in the delay slot of the first call
 * differing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit12ThenInvokeHelperScenarioScena0300_801FC54C(s32 arg0, s32 arg1) {
  func_8015B580(arg1, 0x12);
  func_801FC5A4();
}
