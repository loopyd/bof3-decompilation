#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FA70C
 * @behavior Overlay per-record callback: it first runs the shared front-end
 * startup helper func_8015C088 (0x8015C088), then requests secondary sub-state
 * 0xA in the shared sub-state byte D_80146875, clears the shared bytes
 * 0x80146865, 0x80146866 and 0x80146867, and requests primary scene state 3 in
 * the shared state byte D_80146874. Takes no arguments and returns nothing; its
 * 0x18-byte frame only keeps $ra across the call, and each byte store
 * materialises the 0x8014 page in $at on its own. The constant 0xA is
 * materialised in $v0 straight after the call and the constant 3 in the same
 * register before the three clearing stores, so the store order D_80146875,
 * 0x80146865, 0x80146866, 0x80146867, D_80146874 is the source statement order.
 * No jal or j in the payload reaches the address (a word scan of the 17304-byte
 * image finds none); its only image word is 0x801FAF34, entry 4 of the 29-word
 * overlay callback table D_801FAF24 that
 * dispatchRecordCallbackScenarioScena1100_801FA5B8 (0x801FA5B8) indexes with a
 * record's byte field 0x7A, so the record byte value 4 selects this handler.
 * Its 80 bytes match the span of
 * setSubstate1EClearSharedStateBytesAndRequestPrimaryState8ScenarioScena1200_801FC784
 * (0x801FC784) word for word apart from the two addiu immediates that
 * materialise 0xA and 3 here and 0x1E and 8 there, so that sibling supplied the
 * statement order, operand forms and declaration style while both constants
 * come from this span's own bytes. It differs from its immediate neighbours
 * clearSharedStateBytesThenRequestPrimaryState5ScenarioScena1100_801FA6C0
 * (0x801FA6C0) and
 * clearSharedStateBytesThenRequestPrimaryState6ScenarioScena1100_801FA75C
 * (0x801FA75C) by requesting a sub-state instead of clearing that byte. The
 * name records the proven behavior and the address anchor keeps it
 * target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate0AClearSharedStateBytesAndRequestPrimaryState3ScenarioScena1100_801FA70C(void) {
  func_8015C088();
  D_80146875 = 0xA;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 3;
}
