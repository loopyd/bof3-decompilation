#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FE0E0
 * @behavior Overlay per-record callback: it clears the shared scenario
 * progress byte g_ScenarioProgress (0x80146864), runs the shared front-end
 * startup helper func_8015C088, requests primary scene state 0x0A with
 * secondary sub-state 0 by storing 0x0A in the shared state byte D_80146874
 * and 0 in the shared sub-state byte D_80146875, and finally sets flag bit
 * 0x13 of the shared scenario flag word D_8014686C through func_8015B580. It
 * ignores the record pointer and the second argument its caller passes, reads
 * no other memory and writes none; the 0x18-byte frame only keeps $ra across
 * its two calls and nothing sets $v0 as a result.
 *
 * Name evidence: the leading phrases are the accepted exact sibling wording of
 * clearProgressRequestPrimaryState7AndArmCountdownScenarioScena0800_801FE058
 * (0x801FE058), which clears the same g_ScenarioProgress, runs the same startup
 * helper and stores 7 in the same D_80146874; the stored 0x0A is the value in
 * the emitted `addiu v0,zero,0xA` and the 0x13 is the constant in the `jal`
 * delay slot of func_8015B580, the shared scenario-flag bit helper this
 * repository names as `setScenarioFlagBit13…` for the identical call
 * (setScenarioFlagBit13ThenInvokeHelperScenarioScena0300_801FC578). The stored 0
 * in D_80146875 is the clear the sibling omits from its name in the same way.
 *
 * Table evidence: the address is entry 2 (the word at 0x801FE9C8 reading
 * 0x801FE0E0) of the eleven-entry callback-pointer table at 0x801FE9C0 that
 * dispatchRecordCallbackScenarioScena0800_801FE018 (0x801FE018) selects with the
 * record's unsigned byte at offset 0x7A; no jal in the image targets it, so the
 * table word is its only entry. It is the immediate sibling of entry 0
 * clearProgressRequestPrimaryState7AndArmCountdownScenarioScena0800_801FE058 and
 * entry 1 clearProgressThenRequestPrimaryState7Substate10ScenarioScena0800_801FE0A0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressRequestPrimaryState0AAndSetFlagBit13ScenarioScena0800_801FE0E0(void) {
  g_ScenarioProgress = 0;
  func_8015C088();
  D_80146874 = 0xA;
  D_80146875 = 0;
  func_8015B580(D_8014686C, 0x13);
}
