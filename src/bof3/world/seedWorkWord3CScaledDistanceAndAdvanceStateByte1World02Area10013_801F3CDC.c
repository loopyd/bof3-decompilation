#include "bof3/bof3.h"

/* Shared published work-record pointer cell of the scratchpad. The original
 * materialises the cell address twice, so the post-call group keeps its own
 * read of the record pointer. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

extern s16 func_8015477C(s32 x, s32 z); /* @source 0x8015477C */

/* @source 0x801F3CDC
 * @behavior Step handler: entry 0 of the handler table based at 0x801F4F68 that
 *           this target's dispatcher
 *           dispatchStateByte1Table4f68World02Area10013_801F3C98 indexes with
 *           the work record's state byte at +1. Seeds the record's signed word
 *           at +0x3C with the shared distance helper 0x8015477C of the record
 *           word pair at +0x34/+0x38, scaled by 0x10000, and then advances that
 *           state byte so the next frame dispatches entry 1 at 0x801F3D28,
 *           which copies +0x34/+0x38/+0x3C into a three-word array and hands
 *           the array to the overlay draw routine at 0x801F3D70. Takes no
 *           arguments and returns nothing; its 0x18-byte frame only keeps $ra
 *           across the distance call.
 *           Shape evidence: the seed step of the 4084 handler table at
 *           0x801F30B4, which is still on its asm row, is this span's own
 *           disassembly virtually instruction for instruction - same 0x18 frame,
 *           same scratchpad read, same distance call with an unfilled delay
 *           slot, same scaled store into +0x3C and same state-byte advance -
 *           except that it also clears the record words at +0x14 and +0x20
 *           before advancing. Its statement order, its register-visible reload
 *           of the record pointer after the call and its byte-advance last are
 *           transcribed here with that pair of zero stores left out and this
 *           span's own offsets and frame-local spelling written in.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWord3CScaledDistanceAndAdvanceStateByte1World02Area10013_801F3CDC(
    void) {
  u8* work;
  u8* tail;
  s16 distance;

  work = D_1F800044;
  distance =
      func_8015477C(FIELD_REF(s32, work, 0x34), FIELD_REF(s32, work, 0x38));
  tail = D_1F800044;
  FIELD_REF(s32, tail, 0x3C) = distance << 16;
  tail[1]++;
}
