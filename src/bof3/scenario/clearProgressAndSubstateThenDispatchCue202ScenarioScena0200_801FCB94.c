#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146875;

void func_8015DF18(u32 cue_id);

/* @source 0x801FCB94
 * @behavior Clears the four shared scenario progress bytes at 0x80146864
 * through 0x80146867 and the substate byte at 0x80146875, in that order,
 * then dispatches sound cue 0x202 through the resident helper at 0x8015DF18.
 * Takes no arguments and returns no value. The original saves only the return
 * address in a 0x18-byte frame; the helper consumes the cue in $a0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressAndSubstateThenDispatchCue202ScenarioScena0200_801FCB94(void) {
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  func_8015DF18(0x202u);
}
