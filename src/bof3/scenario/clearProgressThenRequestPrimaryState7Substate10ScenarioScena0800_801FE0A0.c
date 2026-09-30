#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FE0A0
 * @behavior Overlay per-record callback: it clears the shared scenario progress
 * byte g_ScenarioProgress (0x80146864) to zero, runs the shared front-end
 * startup helper func_8015C088 and then requests primary scene state 7 with
 * secondary sub-state 0x0A by storing 7 in the shared state byte D_80146874 and
 * 0x0A in the shared sub-state byte D_80146875. It ignores the record pointer
 * and the second argument its caller passes, reads no other memory and writes
 * none; the 0x18-byte frame only keeps $ra across the call, each of the three
 * byte stores materialises the 0x8014 page in $at on its own, and the two
 * stored constants are materialised in $v0 (`addiu $v0,$zero,imm` feeding each
 * `sb`), not consumed directly as store immediates. The address is entry 1 (the word reading 0x801FE0A0 at 0x801FE9C4) of
 * the eleven-entry callback-pointer table at 0x801FE9C0 that
 * dispatchRecordCallbackScenarioScena0800_801FE018 selects with the record's
 * unsigned byte at offset 0x7A; no in-image jal targets it, so the table word is
 * its only entry. Its shape is the exact sibling
 * requestPrimaryState6Substate10ScenarioScena0700_801FDAFC at 0x801FDAFC, which
 * runs the same startup helper and requests state 6 with sub-state 0x0A, and it
 * additionally clears the scenario progress byte the shared scenario family
 * publishes as g_ScenarioProgress.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressThenRequestPrimaryState7Substate10ScenarioScena0800_801FE0A0(void) {
  g_ScenarioProgress = 0;
  func_8015C088();
  D_80146874 = 7;
  D_80146875 = 0xA;
}
