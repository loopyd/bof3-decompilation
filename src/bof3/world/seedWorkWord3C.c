#include "bof3/bof3.h"

/* Shared published work-record pointer cell of the scratchpad. The original
 * materialises the cell address twice, so the post-call group keeps its own
 * read of the record pointer. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-executable helper. It takes two coordinate words and returns
 * a signed word computed from the height map. */
extern s32 func_8015477C(s32 x, s32 z); /* @source 0x8015477C */

/* @source 0x801F2DD4
 * @behavior Step handler: entry 0 of the two-entry step dispatch table based at
 *           0x801F3B00 that this target's dispatcher
 *           dispatchStateByte1Table3 indexes with
 *           the work record's state byte at +1. Seeds the record word at
 *           +0x3C with the shared helper 0x8015477C applied to the record word
 *           pair at +0x34/+0x38 and shifted left 16 as an unsigned word, then
 *           advances that state byte so the next frame dispatches entry 1 at
 *           0x801F2E20, which copies +0x34/+0x38/+0x3C into a three-word array
 *           and hands the array to the overlay draw routine at 0x801F2E68.
 *           Takes no arguments and returns nothing; its 0x18-byte frame only
 *           keeps $ra across the distance call.
 *           Shape evidence: the 0x4C bytes of this span are byte-identical to
 *           the already-exact sibling
 *           seedWorkWord3CScaledDistanceAndAdvanceStateByte1World02Area10013_801F3CDC
 *           of emi/world02/area100/13 at 0x801F3CDC, proven by one payload cmp
 *           of both spans, which hashed to
 *           1209d84d072b1368b53790b2897a4ceace5f11f180865460ba117085f02d4903.
 *           Its statement order, operand forms, declaration style and post-call
 *           reload of the record pointer are transcribed here with this span's
 *           own offsets written in. The nearer seed
 *           seedWorkWord3CScaledDistanceClearWorkOffsetAndVelocityAndAdvanceStateByte1World02Area10013_801F30B4
 *           is the same body plus two zero stores of the record words at +0x14
 *           and +0x20, which this span does not have. The clone adapter was
 *           declined on this pair: it reports the family spelling FIELD_REF as
 *           an unresolved external, so the body is transcribed instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWord3C(
    void) {
  u8* work;
  u8* tail;
  s32 distance;

  work = D_1F800044;
  distance =
      func_8015477C(FIELD_REF(s32, work, 0x34), FIELD_REF(s32, work, 0x38));
  tail = D_1F800044;
  FIELD_REF(u32, tail, 0x3C) = (u32)distance << 16;
  tail[1]++;
}
