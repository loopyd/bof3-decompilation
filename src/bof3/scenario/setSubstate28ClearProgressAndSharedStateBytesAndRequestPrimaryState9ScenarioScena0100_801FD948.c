#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD948
 * @behavior Entry 14 of this overlay's record callback table at 0x801FE308,
 * whose pointer word sits at 0x801FE340; the overlay dispatcher
 * dispatchRecordCallbackByByte7AScenarioScena0100_801FD444 (0x801FD444)
 * selects it with unsigned byte 0x7A of a record, handing that record in $a0
 * and the address of D_80144E90 in $a1, so the overlay reaches this handler
 * only indirectly and it reads neither argument. It runs the shared front-end
 * startup helper func_8015C088 (0x8015C088), requests secondary sub-state 0x28
 * in the shared sub-state byte D_80146875, clears the shared progress byte
 * g_ScenarioProgress (0x80146864) and the shared state bytes 0x80146865,
 * 0x80146866 and 0x80146867, then requests primary scene state 9 through
 * D_80146874. Takes no arguments and returns nothing. The 0x18-byte frame only
 * keeps $ra across the call, and every byte store materialises the 0x8014 page
 * in $at on its own, which is how the psyq compiler emits separate shared-byte
 * stores. The constant 0x28 is materialised in $v0 straight after the call and
 * the constant 9 reaches $v0 before the four clearing stores, so the store
 * order D_80146875, g_ScenarioProgress, 0x80146865, 0x80146866, 0x80146867,
 * D_80146874 is the source statement order. Neither a jal nor a j in the image
 * targets the address. Its 0x58 bytes are the exact shape of
 * setSubstate14ClearProgressAndSharedStateBytesAndRequestPrimaryState4ScenarioScena0600_801FD014
 * (0x801FD014 of emi/scenario/scena06/00), which differs only in the two stored
 * constants, and it is the same helper-and-clear shape as this target's own
 * clearTwoStateBytesThenRequestPrimaryState11ScenarioScena0100_801FD9A0
 * (0x801FD9A0, the next table entry). The two anchor values 0x28 and 9 are
 * request parameters, so the name keeps the address qualifier.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate28ClearProgressAndSharedStateBytesAndRequestPrimaryState9ScenarioScena0100_801FD948(void) {
  func_8015C088();
  D_80146875 = 0x28;
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 9;
}
