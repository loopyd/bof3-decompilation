#include "bof3/bof3.h"

void func_8015DF18(u16 arg0);

/* @source 0x801FCD38
 * @behavior Queues the front-end cue 0x209 through func_8015DF18 and then returns -1 to the caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 queueCueAndNegativeOneResultScenarioScena1200_801FCD38(void) {
  func_8015DF18(0x209);
  return -1;
}
