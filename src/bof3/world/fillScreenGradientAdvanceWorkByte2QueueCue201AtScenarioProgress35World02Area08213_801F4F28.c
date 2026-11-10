#include "bof3/bof3.h"

extern u8 D_8014933A; /* @source 0x8014933A @kind data */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind data */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */

extern void func_801F5428(s32 color); /* @source 0x801F5428 */
extern void func_8015DF18(u32 cue); /* @source 0x8015DF18 */

/* @source 0x801F4F28
 * @behavior Overlay step-table entry 8 at 0x801F57D0. Clears the shared
 *           palette-stage byte at 0x8014933A and fills the screen with packed
 *           gradient color 0x10001805 through the helper at 0x801F5428.
 *           When scenario progress is 0x35, reads the scratchpad work pointer
 *           at 0x1F800044, advances record dispatch byte +0x02, then queues
 *           sound cue 0x201 through the main-executable dispatcher at
 *           0x8015DF18. Other progress values leave the record unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fillScreenGradientAdvanceWorkByte2QueueCue201AtScenarioProgress35World02Area08213_801F4F28(
    void) {
  u8* work;

  D_8014933A = 0;
  func_801F5428(0x10001805);
  if (g_ScenarioProgress == 0x35) {
    work = workRecordCursor1F800044;
    work[2]++;
    func_8015DF18(0x201);
  }
}
