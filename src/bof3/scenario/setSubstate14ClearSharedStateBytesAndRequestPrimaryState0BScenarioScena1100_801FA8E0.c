#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FA8E0
 * @behavior Overlay per-record callback: it runs the shared front-end startup
 * helper func_8015C088 (0x8015C088), requests secondary sub-state 0x14 in the
 * shared sub-state byte D_80146875, clears the shared main-RAM bytes 0x80146865,
 * 0x80146866 and 0x80146867, and requests primary scene state 0xB in the shared
 * primary state byte D_80146874. It takes no arguments and returns nothing; the
 * 0x18-byte frame only keeps $ra across the call. Each of the five byte stores
 * materialises the 0x8014 page in $at on its own, the constant 0x14 is
 * materialised in $v0 straight after the call to feed the D_80146875 store, and
 * the constant 0xB takes the same register before the three clearing stores, so
 * the store order D_80146875, 0x80146865, 0x80146866, 0x80146867, D_80146874 is
 * the source statement order. No jal and no j anywhere in the payload reaches
 * the address; its only image word is 0x801FAF4C, entry 10 of the 29-word overlay
 * callback run at 0x801FAF24 that dispatchRecordCallbackScenarioScena1100_801FA5B8
 * (0x801FA5B8) indexes with a record byte field at offset 0x7A, so a record byte
 * reading 10 selects this handler. A payload comparison of the 80 bytes here
 * against the span of the exact sibling
 * setSubstate14ClearSharedStateBytesAndRequestPrimaryState0AScenarioScena1100_801FA890
 * (0x801FA890) found every word equal except the one addiu immediate that
 * materialises 0xB here against 0xA there, so that sibling supplied the statement
 * order, operand forms and declaration style while the constant came from this
 * span's own bytes. The name records the proven behavior and the address anchor
 * keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate14ClearSharedStateBytesAndRequestPrimaryState0BScenarioScena1100_801FA8E0(void) {
  func_8015C088();
  D_80146875 = 0x14;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 0xB;
}
