#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC640
 * @behavior Overlay per-record callback: it runs the shared front-end startup
 * helper func_8015C088 (0x8015C088), requests secondary sub-state 0x14 in the
 * shared sub-state byte D_80146875, clears the shared bytes 0x80146865,
 * 0x80146866 and 0x80146867, and requests primary scene state 4 in the shared
 * state byte D_80146874. It takes no arguments and returns nothing; the
 * 0x18-byte frame only keeps $ra across the call. Each byte store materialises
 * the 0x8014 page in $at on its own, the constant 0x14 is materialised in $v0
 * straight after the call to feed the D_80146875 store, and the constant 4 is
 * materialised in the same register before the three clearing stores, so the
 * store order D_80146875, 0x80146865, 0x80146866, 0x80146867, D_80146874 is the
 * source statement order. The only in-image reference to this address is the
 * word at 0x801FD210, entry 9 of the 15-entry overlay record-callback run at
 * 0x801FD1EC that dispatchRecordCallbackByByte7AScenarioScena1200_801FC574
 * (0x801FC574) indexes with a record's unsigned byte at offset 0x7A, so the
 * overlay reaches this callback with the byte reading 9; the record pointer and
 * second argument that dispatcher passes are ignored and no in-image jal
 * targets the address. Its 80 bytes are the shape of the exact sibling
 * setSubstate1EClearSharedStateBytesAndRequestPrimaryState2ScenarioScena0200_801FD4EC
 * (0x801FD4EC of emi/scenario/scena02/00), which runs the same helper with the
 * same five stores and the same order, differing only in the two stored
 * constants (0x1E and 2 there); the neighbouring word 10 of the same run,
 * func_801FC690 (0x801FC690), is the same shape with primary state 5. The name
 * records the proven behavior and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate14ClearSharedStateBytesAndRequestPrimaryState4ScenarioScena1200_801FC640(void) {
  func_8015C088();
  D_80146875 = 0x14;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 4;
}
