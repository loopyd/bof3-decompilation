#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FA840
 * @behavior Overlay per-record callback: it runs the shared front-end startup
 * helper func_8015C088 (0x8015C088), requests secondary sub-state 5 in the
 * shared sub-state byte D_80146875, clears the shared main-RAM bytes 0x80146865,
 * 0x80146866 and 0x80146867, and requests primary scene state 9 in the shared
 * primary state byte D_80146874. It takes no arguments and returns nothing; the
 * 0x18-byte frame only keeps $ra across the call. Each of the five byte stores
 * materialises the 0x8014 page in $at on its own, the constant 5 is materialised
 * in $v0 straight after the call and the constant 9 takes the same register
 * before the three clearing stores, so the store order D_80146875, 0x80146865,
 * 0x80146866, 0x80146867, D_80146874 is the source statement order. No jal and
 * no j anywhere in the payload reaches the address; its only image word is
 * 0x801FAF44, entry 8 of the 29-word overlay callback run at 0x801FAF24 that
 * dispatchRecordCallbackScenarioScena1100_801FA5B8 (0x801FA5B8) indexes with a
 * record byte field at offset 0x7A, so the record byte value 8 selects this
 * handler. A payload comparison of the 80 bytes at 0x801FA840 against the span
 * of setSubstate0AClearSharedStateBytesAndRequestPrimaryState3ScenarioScena1100_801FA70C
 * (0x801FA70C) found every word equal except the two addiu immediates that
 * materialise 5 and 9 here against 0xA and 3 there, so that sibling supplied the
 * statement order, operand forms and declaration style while both constants came
 * from this span's own bytes. The name records the proven behavior and the
 * address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate5ClearSharedStateBytesAndRequestPrimaryState9ScenarioScena1100_801FA840(void) {
  func_8015C088();
  D_80146875 = 5;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 9;
}
