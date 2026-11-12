#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC784
 * @behavior Overlay per-record callback: it runs the shared front-end startup
 * helper func_8015C088 (0x8015C088), requests secondary sub-state 0x1E in the
 * shared sub-state byte D_80146875, clears the shared main-RAM bytes
 * 0x80146865, 0x80146866 and 0x80146867, and requests primary scene state 8 in
 * the shared primary state byte D_80146874. It takes no arguments, reads no
 * state and returns nothing; the 0x18-byte frame only keeps $ra across the
 * call, and each of the five byte stores materialises the 0x8014 page in $at on
 * its own, so the store order D_80146875, 0x80146865, 0x80146866, 0x80146867,
 * D_80146874 is the source statement order. The constant 0x1E is materialised
 * in $v0 straight after the call and the constant 8 in the same register
 * before the three clearing stores. No in-image jal targets the address; its
 * only image word is 0x801FD220, entry 13 of the 15-word overlay callback
 * table at 0x801FD1EC that dispatchRecordCallbackByByte7AScenarioScena1200_801FC574
 * (0x801FC574) indexes with a record's byte field 0x7A, so the record byte
 * value 0x0D selects this handler. Its 80 bytes differ from the exact scena02
 * sibling
 * setSubstate1EClearSharedStateBytesAndRequestPrimaryState2ScenarioScena0200_801FD4EC
 * (0x801FD4EC) in the single `addiu $v0,$zero,0x8` word, which materialises 2
 * there; every other word of the two spans is equal, so that sibling supplied
 * the statement order, operand forms and declaration style while the constants
 * come from this span's own bytes. The neighbouring word 12 of the same table,
 * the exact
 * clearSharedStateBytesThenRequestPrimaryState8ScenarioScena1200_801FC738
 * (0x801FC738), clears D_80146875 instead of requesting sub-state 0x1E. The
 * name records the proven behavior and the address anchor keeps it
 * target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate1EClearSharedStateBytesAndRequestPrimaryState8ScenarioScena1200_801FC784(void) {
  func_8015C088();
  D_80146875 = 0x1E;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 8;
}
