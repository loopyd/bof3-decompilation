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

/* @source 0x801FD11C
 * @behavior Runs helper func_8015C088, clears the progress byte at 0x80146864
 * and shared bytes 0x80146865..0x80146867, clears the secondary sub-state byte
 * D_80146875, then requests primary state 0xA through D_80146874. The constant
 * is materialised once in $v0 and reused by that store, the 0x18-byte frame
 * only keeps $ra across the call, and the two arguments the record dispatcher
 * passes (the record pointer and the shared D_8014686C word) are ignored, so
 * the function takes none and returns nothing. The address is reached only
 * through the overlay's per-record callback table at 0x801FE42C: its only image
 * word is word 10 of that table, at 0x801FE454, and no in-image jal targets it.
 * Its 84 bytes are instruction-for-instruction identical to the exact siblings
 * at 0x801FCE18 (PrimaryState1), 0x801FCE6C (PrimaryState2) and 0x801FCFC0
 * (PrimaryState4), which differ only in the immediate of the request store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressAndSharedStateBytesThenRequestPrimaryState0AScenarioScena0600_801FD11C(void) {
  func_8015C088();
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 0xA;
}
