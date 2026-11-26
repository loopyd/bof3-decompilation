#include "bof3/bof3.h"

extern u8* scratchpadWorkRecord; /* @source 0x1F800044 @kind data */
extern void func_8015DF18(u16 cue); /* @source 0x8015DF18 */

/* @source 0x801F3D4C
 * @behavior First handler in the table at 0x801F4634, indexed by work byte 2.
 * Clears work byte 9, queues cue 0x202 through 0x8015DF18, then re-reads the
 * scratchpad work-record pointer and increments work byte 2. Takes no arguments
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWorkQueueAdvance(void) {
  u8* work;

  scratchpadWorkRecord[9] = 0;
  func_8015DF18(0x202);
  work = scratchpadWorkRecord;
  work[2] += 1;
}
