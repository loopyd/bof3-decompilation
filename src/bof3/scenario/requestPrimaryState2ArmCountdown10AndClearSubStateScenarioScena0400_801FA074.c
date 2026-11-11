#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;

void func_8015C088(void);

/* @source 0x801FA074
 * @behavior Overlay state-request handler of the four-word handler table at
 * 0x801FA230: it runs the shared front-end startup helper func_8015C088
 * (0x8015C088), arms the shared unsigned halfword frame countdown D_80146876
 * with 10, requests primary front-selector state 2 by storing 2 in the shared
 * state byte D_80146874 and leaves the shared sub-state byte D_80146875 at 0,
 * in that store order. It reads no other memory and writes none; the 0x18-byte
 * frame only keeps $ra across the call, each of the three stores materialises
 * the 0x8014 page in $at on its own, the two non-zero constants are
 * materialised in $v0 (addiu $v0, $zero, imm feeding each store) while the zero
 * store uses $zero directly, and nothing sets $v0 as a result. No in-image jal
 * targets this address: its only image word is 0x801FA074 at 0x801FA230, entry
 * 0 of the four-word code-pointer run 0x801FA074, 0x801FA0B4, 0x801FA12C,
 * 0x801FA178 that func_801FA010 (0x801FA010) indexes with the sign-extended
 * byte func_801A7AD8(&D_801FA228, 1, ...) returns, calling the selected word
 * and reporting 1, or reporting -1 when that byte is negative. Its 64 bytes
 * carry the shape of the exact sibling lift
 * clearProgressRequestPrimaryState7AndArmCountdownScenarioScena0800_801FE058
 * (0x801FE058), which runs the same helper and stores the same three cells; the
 * constants, the store order and the pushed countdown here come from this
 * span's own bytes, and no extracted payload holds a byte-identical copy of it.
 * The address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState2ArmCountdown10AndClearSubStateScenarioScena0400_801FA074(void) {
  func_8015C088();
  D_80146876 = 10;
  D_80146874 = 2;
  D_80146875 = 0;
}
