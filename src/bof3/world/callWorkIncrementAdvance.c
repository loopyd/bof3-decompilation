#include "bof3/bof3.h"

extern u8* scratchpadWorkRecord; /* @source 0x1F800044 @kind data */
extern void func_801F3F04(s32 arg0); /* @source 0x801F3F04 */

/* @source 0x801F3D90
 * @behavior Second handler in the table at 0x801F4634, indexed by work byte 2.
 * Calls 0x801F3F04 with work byte 9, then increments that byte. When the
 * stored byte reaches 8, clears it and advances work byte 2. Re-reads the
 * scratchpad work-record pointer after the call and each potentially aliasing
 * byte store. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkIncrementAdvance(void) {
  u8* work;
  u8* step;
  u8* tail;

  func_801F3F04(scratchpadWorkRecord[9]);
  work = scratchpadWorkRecord;
  work[9] += 1;
  step = scratchpadWorkRecord;
  if (step[9] >= 8) {
    step[9] = 0;
    tail = scratchpadWorkRecord;
    tail[2] += 1;
  }
}
