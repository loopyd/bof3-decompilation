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

/* @source 0x801FCE18
 * @behavior Runs helper func_8015C088, clears the progress byte at 0x80146864
 * and shared bytes 0x80146865..0x80146867, clears the secondary state byte
 * D_80146875, then requests primary state 1 through D_80146874. This is word
 * 1 of the overlay's record callback table at 0x801FE42C; it ignores the
 * record pointer and shared-word arguments supplied by the dispatcher and
 * returns no value. The six byte stores follow the helper in that order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressAndSharedStateBytesThenRequestPrimaryState1ScenarioScena0600_801FCE18(void) {
  func_8015C088();
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 1;
}
