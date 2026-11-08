#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FC4AC
 * @behavior Handler slot 0 of the overlay's nine-pointer per-record callback
 * table at 0x801FD00C (D_801FD00C, word 0). The frame dispatcher at 0x801FC46C
 * selects a slot from the record's unsigned byte at offset 0x7A and calls the
 * handler as (record, D_8014686C), so this handler receives the record pointer
 * in $a0 and the shared scenario flag word D_8014686C (0x8014686C) in $a1; it
 * reads no field of the record. It runs the shared front-end startup helper
 * func_8015C088 (0x8015C088) first, sets flag bit index 0x14 of that flag word
 * through the shared bit-set helper func_8015B580 (0x8015B580), and then
 * requests primary scene state 4 with a cleared sub-state by storing 4 in the
 * shared state byte D_80146874 (0x80146874) and 0 in the shared sub-state byte
 * D_80146875 (0x80146875). Nothing sets $v0 for a result, the 0x18-byte frame
 * only keeps $ra and the saved $a1 across the two calls, and each byte store
 * materialises the 0x8014 page in $at on its own.
 *
 * Name evidence: the flag-word helper call is the same
 * `func_8015B580(arg1, const)` shape the accepted exact siblings
 * setScenarioFlagBit10ThenInvokeHelperScenarioScena0300_801FC4F4
 * (0x801FC4F4, const 0x10), setScenarioFlagBit11ThenInvokeHelperScenarioScena0300_801FC520
 * (0x801FC520, const 0x11), setScenarioFlagBit12ThenInvokeHelperScenarioScena0300_801FC54C
 * (0x801FC54C, const 0x12) and setScenarioFlagBit13ThenInvokeHelperScenarioScena0300_801FC578
 * (0x801FC578, const 0x13) carry, so 0x14 continues that wording as flag bit
 * 0x14; the stored 4 is the emitted `addiu v0,zero,0x4` and the cleared 0 is
 * the emitted `sb $zero`, the pair the same repository names
 * "RequestPrimaryState4Substate0" in
 * requestPrimaryState5Substate0AndQueueFrontEndMode12ScenarioScena0300_801FC9DC
 * (0x801FC9DC). The address anchor keeps the name target-qualified.
 *
 * Table evidence: the address is word 0 of the nine-entry callback-pointer run
 * at 0x801FD00C, and the only reader of that run is
 * dispatchRecordCallbackScenarioScena0300_801FC46C (0x801FC46C); no jal in the
 * image targets 0x801FC4AC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit14ThenRequestPrimaryState4Substate0ScenarioScena0300_801FC4AC(s32 arg0, u32 arg1) {
  func_8015C088();
  func_8015B580(arg1, 0x14);
  D_80146874 = 4;
  D_80146875 = 0;
}
