#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FAA24
 * @behavior Overlay per-record callback: it runs the shared front-end startup
 * helper func_8015C088 (0x8015C088), requests secondary sub-state 0x28 in the
 * shared sub-state byte D_80146875, clears the shared main-RAM bytes 0x80146865,
 * 0x80146866 and 0x80146867, and requests primary scene state 5 in the shared
 * primary state byte D_80146874. It takes no arguments and returns nothing; the
 * 0x18-byte frame only keeps $ra across the call. Each of the five byte stores
 * materialises the 0x8014 page in $at on its own, the constant 0x28 is
 * materialised in $v0 straight after the call to feed the D_80146875 store, and
 * the constant 5 takes the same register before the three clearing stores, so
 * the store order D_80146875, 0x80146865, 0x80146866, 0x80146867, D_80146874 is
 * the source statement order. No jal and no j anywhere in the payload reaches
 * the address; its only image word is 0x801FAF5C, entry 14 of the 29-word overlay
 * callback run at 0x801FAF24 that dispatchRecordCallbackScenarioScena1100_801FA5B8
 * (0x801FA5B8) indexes with a record byte field at offset 0x7A, so a record byte
 * reading 14 selects this handler. A payload comparison of the 80 bytes here
 * against the span of the exact sibling
 * setSubstate14ClearSharedStateBytesAndRequestPrimaryState0BScenarioScena1100_801FA8E0
 * (0x801FA8E0) found every word equal except the two addiu immediates that
 * materialise 0x28 and 5 here against 0x14 and 0xB there, so that sibling
 * supplied the statement order, operand forms and declaration style while both
 * constants came from this span's own bytes. The name records the proven
 * behavior and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate28ClearSharedStateBytesAndRequestPrimaryState5ScenarioScena1100_801FAA24(void) {
  func_8015C088();
  D_80146875 = 0x28;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 5;
}
