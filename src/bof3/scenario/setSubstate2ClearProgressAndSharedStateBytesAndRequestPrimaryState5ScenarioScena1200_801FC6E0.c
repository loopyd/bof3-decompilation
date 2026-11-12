#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC6E0
 * @behavior Overlay per-record callback and entry 11 (the word at 0x801FD218)
 * of the 15-word record-callback table at 0x801FD1EC that
 * dispatchRecordCallbackByByte7AScenarioScena1200_801FC574 (0x801FC574)
 * selects with the record unsigned byte at offset 0x7A: it runs the shared
 * front-end startup helper func_8015C088 (0x8015C088), requests secondary
 * sub-state 2 in the shared sub-state byte D_80146875, clears the shared
 * progress byte g_ScenarioProgress (0x80146864) and the shared state bytes
 * 0x80146865, 0x80146866 and 0x80146867, then requests primary scene state 5
 * by storing 5 in the shared primary state byte D_80146874. It takes no
 * arguments, ignores the record pointer and second argument its dispatcher
 * passes, reads no other state and writes nothing else. The 0x18-byte frame
 * only keeps $ra across the call, and every byte store materialises the 0x8014
 * page in $at on its own. The sub-state constant is materialised in $v0
 * straight after the call and the primary-state constant reaches the same $v0
 * ahead of the four clearing stores, so the store order D_80146875,
 * g_ScenarioProgress, 0x80146865, 0x80146866, 0x80146867, D_80146874 is the
 * source statement order. Neither a jal nor a j in the payload targets the
 * address, and the table word is its only reference. Its 0x58 bytes carried the
 * same statement order and operand forms as
 * setSubstate0AClearProgressAndSharedStateBytesAndRequestPrimaryState0EScenarioScena0600_801FD170
 * (0x801FD170 of emi/scenario/scena06/00) when that span was lifted, and a byte
 * comparison of the two payload spans showed them differing in exactly the two
 * stored constants (0xA and 0xE there); this body writes its own 2 and 5. The
 * name records that evidenced behavior and the address anchor keeps it
 * target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate2ClearProgressAndSharedStateBytesAndRequestPrimaryState5ScenarioScena1200_801FC6E0(void) {
  func_8015C088();
  D_80146875 = 2;
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 5;
}
