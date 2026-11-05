#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD4EC
 * @behavior Overlay state-request handler: it first runs the shared front-end
 * startup helper func_8015C088 (0x8015C088), then requests secondary sub-state
 * 0x1E in the shared sub-state byte D_80146875, clears the shared bytes
 * 0x80146865, 0x80146866 and 0x80146867, and requests primary scene state 2 in
 * the shared state byte D_80146874. Takes no arguments and returns nothing;
 * its 0x18-byte frame only keeps $ra across the call, and each byte store
 * materialises the 0x8014 page in $at on its own, which is how the psyq
 * compiler emits separate shared-byte stores. The constant 0x1E is
 * materialised in $v0 straight after the call and the constant 2 in the same
 * register before the three clearing stores, so the store order D_80146875,
 * 0x80146865, 0x80146866, 0x80146867, D_80146874 is the source statement
 * order. No in-image jal targets the address; its only image word is
 * 0x801FE350, entry 19 of the overlay callback table D_801FE304 that
 * dispatchRecordCallbackByByte7AScenarioScena0200_801FD084 (0x801FD084)
 * indexes with a record's byte field 0x7A, so the overlay reaches this handler
 * only indirectly. Its 80 bytes are the shape of the exact sibling lifts
 * setProgress1ClearSharedBytes6566AndRequestPrimaryState7ScenarioScena0200_801FD2E0
 * (0x801FD2E0) and clearSharedStateBytesThenRequestPrimaryState3ScenarioScena1100_801FA674
 * (0x801FA674), differing in the stored constants and in additionally clearing
 * 0x80146867. The address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate1EClearSharedStateBytesAndRequestPrimaryState2ScenarioScena0200_801FD4EC(void) {
  func_8015C088();
  D_80146875 = 0x1E;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 2;
}
