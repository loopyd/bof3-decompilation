#include "bof3/bof3.h"

extern u8 D_8014933A;
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */

/* Overlay screen-gradient helper; each packed five-bit channel becomes 0xF8
 * when the supplied word is -1. */
extern void func_801F5428(s32 color);

/* Main-executable cue dispatcher, also used by this overlay's other steps. */
extern void func_8015DF18(u16 cue);

/* @source 0x801F50C4
 * @behavior Step-table callback at 0x801F57FC. Clears the shared palette-stage
 *           byte at 0x8014933A, fills the 0x140 by 0xF0 screen with white
 *           through the gradient helper at 0x801F5428 using packed word -1,
 *           then queues cues 0x202 and 0x200 through 0x8015DF18 in that order.
 *           Reads the scratchpad work-record pointer cell at 0x1F800044 to
 *           clear record byte +0x09, then reads it again and increments
 *           record byte +0x02. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fillScreenWhiteQueueCues202And200ClearWorkByte9AdvanceWorkByte2World02Area08213_801F50C4(
    void) {
  D_8014933A = 0;
  func_801F5428(-1);
  func_8015DF18(0x202);
  func_8015DF18(0x200);
  workRecordCursor1F800044[9] = 0;
  workRecordCursor1F800044[2]++;
}
