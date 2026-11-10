#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Every store through the record re-reads
 * this cell, because a byte store through the record may alias the cell. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Local polygon helper that emits the four texture-mapped quads of the work
 * record's position band. */
extern void func_801F2EFC(void); /* @source 0x801F2EFC */

/* Local polygon helper that emits the work record's quad pair at an offset of
 * sixteen from the position word. */
extern void func_801F3064(void); /* @source 0x801F3064 */

/* @source 0x801F32FC
 * @behavior Overlay per-frame work-record step: increments the tick byte at
 * 0x09 of the work record published at the scratchpad pointer cell 0x1F800044,
 * re-reads that cell because a store through the record may alias it, and
 * stores 0xFFFFD480 plus eight times the low bit of the tick byte into the
 * record's position word at 0x34, so consecutive ticks alternate between
 * 0xFFFFD480 and 0xFFFFD488. On the tick that reaches 8 the same path clears
 * the tick byte, stores 0xFFFFD480 into the position word again and bumps the
 * state byte at 0x02, the byte the overlay's state-byte-2 dispatcher uses as
 * its handler index. Both paths then call the two local polygon helpers at
 * 0x801F2EFC and 0x801F3064. Takes no arguments and returns nothing; the frame
 * of 0x18 bytes holds the return address only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkTickAlternatePositionWordByTickParityThenStepStateAndDrawPolygonsWorld01Area04113_801F32FC(void) {
  u8* work;
  u8* step;
  u8* tail;
  u8 state;
  s32 position;

  work = D_1F800044;
  work[9] = work[9] + 1;
  step = D_1F800044;
  position = ((step[9] & 1) << 3) - 0x2B80;
  *(s32*)(step + 0x34) = position;
  if (step[9] >= 8) {
    step[9] = 0;
    tail = D_1F800044;
    state = tail[2];
    *(s32*)(tail + 0x34) = -0x2B80;
    tail[2] = state + 1;
  }
  func_801F2EFC();
  func_801F3064();
}
