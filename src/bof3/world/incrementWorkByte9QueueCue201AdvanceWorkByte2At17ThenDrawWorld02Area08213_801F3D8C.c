#include "bof3/bof3.h"

extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */
extern void func_8015DF18(u32 cue); /* @source 0x8015DF18 */
extern void func_801F4430(void); /* @source 0x801F4430 */

/* @source 0x801F3D8C
 * @behavior Entry 3 of the work-byte-2 handler table at 0x801F576C.
 *           Increments scratch work-record byte +0x09 and re-reads the work
 *           pointer to test that byte against the unsigned bound 17. At or
 *           above that bound, clears the counter, queues cue 0x201 through
 *           0x8015DF18, then re-reads the work pointer and advances dispatch
 *           byte +0x02. Always calls the draw helper at 0x801F4430.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9QueueCue201AdvanceWorkByte2At17ThenDrawWorld02Area08213_801F3D8C(
    void) {
  u8* work;
  u8* tick;
  u8* step;

  work = workRecordCursor1F800044;
  work[9]++;
  tick = workRecordCursor1F800044;
  if (tick[9] >= 0x11u) {
    tick[9] = 0;
    func_8015DF18(0x201);
    step = workRecordCursor1F800044;
    step[2]++;
  }
  func_801F4430();
}
