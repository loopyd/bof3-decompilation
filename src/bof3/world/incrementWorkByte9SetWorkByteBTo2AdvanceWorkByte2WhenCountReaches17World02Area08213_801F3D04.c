#include "bof3/bof3.h"

/* Scratch work-record pointer cell published by the overlay. The original
 * re-reads it once per access group, re-materializing its address
 * (lui $vN,0x1F80 / lw $vN,68($vN)) for each read, so it is named through this
 * target's map instead of the fixed-address accessor. */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */

/* Draw helper, the overlay's own routine at 0x801F4430. Its body rotates and
 * projects the work record fields +0x30/+0x32/+0x34, draws a run of four
 * textured quads, and, when the record's +0x0B byte is non-zero, hands that
 * byte plus 0x201 to the cue dispatcher func_8015DF18 and then clears the byte.
 * Splat binds the address through this target's generated asm label. */
void func_801F4430(void);

/* @source 0x801F3D04
 * @behavior Overlay per-frame handler, entry index 2 of the eight-pointer
 *           byte-2 handler table at 0x801F576C, so the byte +0x02 handler
 *           advance below passes control to the following table entry at
 *           0x801F3D8C. It advances the scratch work record's +0x09 byte
 *           counter by one and tests the incremented value against 0x11 as an
 *           unsigned bound: while it stays below 17 the handler only draws.
 *           Once the counter reaches 17 it clears +0x09, writes the byte 2 into
 *           the record's +0x0B field and advances the record's +0x02 dispatch
 *           byte by one. The +0x0B store arms the pending cue index that the
 *           draw helper at 0x801F4430 consumes on this same frame, handing
 *           0x201 plus that index to the cue dispatcher func_8015DF18 before
 *           clearing the field; the immediately preceding sibling handler
 *           0x801F3C7C arms the same field with 1 and the following one
 *           0x801F3D8C raises cue 0x201 directly instead. The counter and the
 *           handler test share the pointer loaded before the comparison, while
 *           the +0x0B store and the +0x02 advance each re-read the work cell.
 *           Every call ends by the draw helper at 0x801F4430; the function has
 *           a 0x18-byte frame holding the saved return address at 0x10, takes
 *           no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9SetWorkByteBTo2AdvanceWorkByte2WhenCountReaches17World02Area08213_801F3D04(
    void) {
  u8* work;
  u8* tick;
  u8* cue;
  u8* step;

  work = workRecordCursor1F800044;
  work[9]++;
  tick = workRecordCursor1F800044;
  if (tick[9] >= 0x11u) {
    tick[9] = 0;
    cue = workRecordCursor1F800044;
    cue[0x0B] = 2;
    step = workRecordCursor1F800044;
    step[2]++;
  }
  func_801F4430();
}
