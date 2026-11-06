#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x8015477C @kind unknown
 * Shared distance helper over the word pair passed as (x, z); callers publish
 * its result scaled by 0x10000.
 */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* @source 0x801F4078
 * @behavior Overlay work-byte-1 handler, entry 0 of the local handler table at
 *           0x801F6270: reads the work pointer from scratchpad slot 0x1F800044,
 *           stores the position pair 0x740000/0x60000 in its words at offsets
 *           0x34 and 0x38, then publishes the func_8015477C distance of that
 *           pair, scaled by 0x10000, in its word at offset 0x3C. It reloads the
 *           work pointer after the call, arms the countdown word at offset 0x0C
 *           with 300 and advances the dispatch byte at offset 0x01, handing the
 *           step over to the next handler of the same table. Takes no arguments
 *           and returns nothing. Field roles stay offset-anchored beyond what
 *           the neighbouring handlers read.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The frame slot below the pointers is a reconstruction of a declaration the
 * original translation unit carried but this overlay's emitted code never
 * references: the original frame is 0x28 with vars=16 while this function's
 * locals need vars=0, it saves only $ra at 0x20, and no instruction in the
 * original body reads or writes the 16 bytes at 0x10-0x1F (every emitted $sp
 * offset is 0x20 for $ra). Declaring one 16-byte local restores that
 * reservation exactly; the name and type are a reconstruction choice
 * constrained only by the slot size. Same recorded shape as the frame slot in
 * func_801F7670. Flagged for independent review; removing the declaration
 * leaves only the frame-size/offset residual (19/23 instructions, 92 bytes).
 */
void seedWorkDistancePairArmCountdownAndAdvanceStepWorld02Area07714_801F4078(
    void) {
  u32 unusedFrameSlot[4];
  u8* work;
  u8* next;
  s16 distance;

  (void)unusedFrameSlot;
  work = workRecordCursor1F800044;
  *(u32*)(work + 0x34) = 0x740000;
  *(u32*)(work + 0x38) = 0x60000;
  distance = func_8015477C(*(s32*)(work + 0x34), 0x60000);
  next = workRecordCursor1F800044;
  *(u32*)(next + 0x3C) = distance << 16;
  *(u32*)(next + 0x0C) = 300;
  next[1]++;
}
