#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;

void func_8015C088(void);

/* @source 0x801FE058
 * @behavior Overlay per-record callback: it clears the shared scenario progress
 * byte g_ScenarioProgress (0x80146864) to zero, runs the shared front-end
 * startup helper func_8015C088 and then requests primary front-selector state 7
 * by storing 7 in the shared state byte D_80146874, arms the shared unsigned
 * halfword countdown D_80146876 with 0x20 and leaves the shared sub-state byte
 * D_80146875 at 0 (stores 7 to D_80146874, then 0x20 to D_80146876, then 0 to
 * D_80146875, in that order). It ignores the record pointer and the second
 * argument its caller passes, reads no other memory and writes none; the
 * 0x18-byte frame only keeps $ra across the call, each of the three stores
 * materialises the 0x8014 page in $at on its own, the two non-zero constants
 * are materialised in $v0 (addiu $v0, $zero, imm feeding each store) while the
 * zero store uses $zero directly, and nothing sets $v0 as a result. The address
 * is entry 0 (the word at 0x801FE9C0 reading 0x801FE058) of the eleven-entry
 * callback-pointer table at 0x801FE9C0 that
 * dispatchRecordCallbackScenarioScena0800_801FE018 selects with the record's
 * unsigned byte at offset 0x7A; no jal in the image targets it, so the table
 * word is its only entry. It is the immediate sibling of entry 1
 * clearProgressThenRequestPrimaryState7Substate10ScenarioScena0800_801FE0A0 at
 * 0x801FE0A0, which clears the same progress byte, runs the same startup helper
 * and requests the same primary state 7 but with sub-state 0x0A and without
 * touching D_80146876. D_80146876 is the shared unsigned halfword frame
 * countdown that the exact scena16 lifts load, decrement and re-arm (0x7F, 0x20,
 * 0x130, 0x200).
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressRequestPrimaryState7AndArmCountdownScenarioScena0800_801FE058(void) {
  g_ScenarioProgress = 0;
  func_8015C088();
  D_80146874 = 7;
  D_80146876 = 0x20;
  D_80146875 = 0;
}
