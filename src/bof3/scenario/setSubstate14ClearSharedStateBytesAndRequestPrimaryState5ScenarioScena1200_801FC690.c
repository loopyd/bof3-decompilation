#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC690
 * @behavior Overlay per-record callback and entry 10 (the word at 0x801FD214) of
 * the 15-word record-callback run at 0x801FD1EC that
 * dispatchRecordCallbackByByte7AScenarioScena1200_801FC574 (0x801FC574) selects
 * with the record unsigned byte at offset 0x7A: it runs the shared front-end
 * startup helper func_8015C088 (0x8015C088), requests secondary sub-state 0x14
 * in the shared sub-state byte D_80146875, clears the shared bytes 0x80146865,
 * 0x80146866 and 0x80146867, and requests primary scene state 5 in the shared
 * state byte D_80146874. It takes no arguments, ignores the record pointer and
 * second argument its dispatcher passes, reads no other state and writes nothing
 * else; the 0x18-byte frame only keeps $ra across the call. Each byte store
 * materialises the 0x8014 page in $at on its own, the constant 0x14 is
 * materialised in $v0 straight after the call to feed the D_80146875 store, and
 * the constant 5 is materialised in the same register before the three clearing
 * stores, so the store order D_80146875, 0x80146865, 0x80146866, 0x80146867,
 * D_80146874 is the source statement order. The only in-image reference to this
 * address is the word at 0x801FD214, entry 10 of that run, and no in-image jal
 * targets the address. Its 80 bytes are the shape of the exact sibling
 * setSubstate14ClearSharedStateBytesAndRequestPrimaryState4ScenarioScena1200_801FC640
 * (0x801FC640 of this target), which runs the same helper with the same five
 * stores in the same order and whose own live gate measured exact, differing only
 * in the one stored primary-state constant 4 there. The name records the proven
 * behavior and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate14ClearSharedStateBytesAndRequestPrimaryState5ScenarioScena1200_801FC690(void) {
  func_8015C088();
  D_80146875 = 0x14;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 5;
}
