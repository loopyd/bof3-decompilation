#include "bof3/bof3.h"

/* Shared published work-record pointer cell of the scratchpad. The original
 * materialises the cell address twice, once for the entry reads and once after
 * the distance call, so the post-call group keeps its own read of the record
 * pointer. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Distance helper of the shared main executable: it answers the distance of the
 * signed 16.16 coordinate pair it is handed as a halfword. */
extern s16 func_8015477C(s32 x, s32 z); /* @source 0x8015477C */

/* @source 0x801F30B4
 * @behavior Step handler: entry 0 of the handler table based at 0x801F4084 that
 *           this target's dispatcher
 *           dispatchStateByte1Table4084World02Area10013_801F3070 indexes with
 *           the work record's state byte at +1. Seeds the record's signed word
 *           at +0x3C with the shared distance helper 0x8015477C of the record
 *           word pair at +0x34/+0x38, scaled by 0x10000, clears the record
 *           words at +0x14 and +0x20, and then advances that state byte so the
 *           next frame dispatches entry 1 at 0x801F3108, which adds 0x00100000
 *           to +0x20, adds that into +0x14 and draws the record's quad from
 *           that advanced value. Takes no arguments and returns nothing; its
 *           0x18-byte frame only keeps $ra across the distance call.
 *           Shape evidence: the seed step of the sibling 4F68 handler table at
 *           0x801F3CDC is the same body with the two zero stores left out -
 *           same 0x18 frame, same scratchpad read, same distance call with an
 *           unfilled delay slot, same reload of the record pointer after that
 *           call, same scaled store into +0x3C and same state-byte advance - so
 *           its statement order, operand forms and declaration style are
 *           transcribed here with this span's own two clearing stores, written
 *           in the order this span's bytes store them.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWord3CScaledDistanceClearWorkOffsetAndVelocityAndAdvanceStateByte1World02Area10013_801F30B4(
    void) {
  u8* work;
  u8* tail;
  s16 distance;

  work = D_1F800044;
  distance =
      func_8015477C(FIELD_REF(s32, work, 0x34), FIELD_REF(s32, work, 0x38));
  tail = D_1F800044;
  FIELD_REF(s32, tail, 0x3C) = distance << 16;
  FIELD_REF(s32, tail, 0x14) = 0;
  FIELD_REF(s32, tail, 0x20) = 0;
  tail[1]++;
}
