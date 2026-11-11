#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Every access re-reads this cell,
 * because a byte store through the record may alias it. */
extern u8* D_1F800044;

/* Local level drawer at 0x801F2F44, still an asm boundary in this target, so
 * only the prototype the call site needs is declared here. */
void func_801F2F44(u32 level);

/* Shared world-marker queue, absent from the shipped headers. */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2D84
 * @behavior Entry 2 of the state-byte-+0x02 dispatch table at 0x801F3930 that
 *           the dispatcher 0x801F2C04 indexes: passes the work record's byte
 *           +0x09 as the single level argument of the local eight-row quad
 *           renderer at 0x801F2F44, then increments that byte. When the
 *           incremented countdown reaches 8 the same path queues one shared
 *           world-marker block through 0x801A4BC0 at the record's halfwords
 *           +0x36 and +0x3A with size 0x8A, clears the countdown and bumps the
 *           dispatch state byte +0x02, handing the next frame to entry 3 of the
 *           same table. The record pointer published by the scratchpad pointer
 *           cell 0x1F800044 is re-read once per store group, as the original
 *           does. Takes no arguments and returns nothing; the 0x18-byte frame
 *           holds only the saved return address at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawCountdownLevelThenQueueMarkerAtWorkHalfwords36And3aSize8aResetCountdownAndAdvanceStepWorld02Area10213_801F2D84(
    void) {
  u8* countdown;
  u8* step;
  u8* reset;
  u8* handler;

  func_801F2F44(D_1F800044[9]);
  countdown = D_1F800044;
  countdown[9]++;
  step = D_1F800044;
  if (step[9] >= 8) {
    func_801A4BC0(*(s16*)(step + 0x36), *(s16*)(step + 0x3A), 0x8A);
    reset = D_1F800044;
    reset[9] = 0;
    handler = D_1F800044;
    handler[2]++;
  }
}
