#include "bof3/bof3.h"

/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;
/* @source 0x80146874 @kind unknown */
extern s8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD06C
 * @behavior Runs the shared front-end startup helper func_8015C088
 * (0x8015C088), sets the shared progress byte g_ScenarioProgress (0x80146864)
 * to 1, clears the shared bytes 0x80146865, 0x80146866 and 0x80146867, clears
 * the secondary sub-state byte D_80146875, then requests primary state 6
 * through D_80146874. Takes no arguments and returns nothing: the record
 * dispatcher dispatchRecordCallbackScenarioScena0600_801FCDD0 (0x801FCDD0)
 * invokes it as (record, D_8014686C) and both arguments are ignored. The
 * 0x18-byte frame only keeps $ra across the call, each byte store materialises
 * the 0x8014 page in $at on its own, and the constant 6 reaches $v0 right after
 * the progress store to feed the last store, so the store order 0x80146864,
 * 0x80146865, 0x80146866, 0x80146867, D_80146875, D_80146874 is the source
 * statement order. The address is reached only through the overlay's per-record
 * callback table at 0x801FE42C: its only image word is word 8 of that table, at
 * 0x801FE44C, and no in-image jal or j targets it. Its 88 bytes are this
 * target's own clearProgressAndSharedStateBytesThenRequestPrimaryState0A
 * handler at 0x801FD11C plus the progress store, and share the
 * helper-and-clear shape of the 0x801FCFC0 handler, which requests primary
 * state 4; they differ in the stored constants.
 * The address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setProgress1ClearSharedStateBytesAndRequestPrimaryState6ScenarioScena0600_801FD06C(void) {
  func_8015C088();
  g_ScenarioProgress = 1;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 6;
}
