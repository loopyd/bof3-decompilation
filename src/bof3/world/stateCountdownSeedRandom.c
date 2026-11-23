#include "bof3/bof3.h"

extern u8 D_801F45C4[];
extern int rand(void);

/* @source 0x801F2C48
 * @behavior Entry handler at index 0 of the byte-1 dispatch table based at
 * 0x801F45CC. Clears the countdown byte at offset 0x09 of the work record
 * published at the scratchpad pointer cell 0x1F800044, then rolls rand() and
 * masks it to the low three bits to index the 8-entry mode table at 0x801F45C4
 * and stores the selected byte into the work record byte at offset 0x01, so the
 * dispatcher at 0x801F2C04 runs the handler that table cell names next. The
 * table bytes are 01 04 04 01 01 04 04 01, so the record moves to state 1
 * (0x801F2C88) or state 4 (0x801F2D6C). Takes no arguments and returns
 * nothing.
 * The shape is the same overlay handler at 0x801F2C88, which also clears the
 * countdown byte at offset 0x09 and arms the state byte at offset 0x01, and the
 * longer state-2 handlers of this overlay, which re-read the same scratchpad
 * cell for every access.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCountdownSeedRandom(void) {
  u8** slots;
  u8* work1;
  u8* work2;
  s32 mode;

  slots = SPAD_PTR_TABLE(u8);
  work1 = slots[0x11];
  work1[0x09] = 0;
  mode = rand() & 7;
  slots = SPAD_PTR_TABLE(u8);
  work2 = slots[0x11];
  work2[0x01] = D_801F45C4[mode];
}
