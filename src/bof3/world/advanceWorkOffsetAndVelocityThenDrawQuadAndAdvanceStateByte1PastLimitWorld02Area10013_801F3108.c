#include "bof3/bof3.h"

void func_801F31E8(s32* args, s32 offset);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer after the draw call. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F3108
 * @behavior Overlay step handler, entry 1 of the handler table based at
 *           0x801F4084 that this target's dispatcher
 *           dispatchStateByte1Table4084World02Area10013_801F3070 indexes with
 *           the work record's state byte at +1; entry 0 at 0x801F30B4 seeds the
 *           fields this body moves by storing the 0x8015477C distance of the
 *           record's +0x34/+0x38 pair, scaled by 0x10000, in +0x3C, clearing
 *           +0x14 and +0x20 and advancing that state byte.
 *           This handler adds 0x00100000 to the work record's signed word at
 *           +0x20, adds that advanced word into the signed word at +0x14,
 *           copies the record's words at +0x34, +0x38 and +0x3C into a
 *           three-word local array, and calls the overlay draw routine at
 *           0x801F31E8 with that array and the advanced +0x14 value, which
 *           draws the record's quad with +0x14 added to its third coordinate.
 *           It then re-reads the cursor cell and, when the word at +0x14
 *           exceeds 0x07FFFFFF, advances the state byte at +1 so the next frame
 *           dispatches entry 2 at 0x801F31A0. Takes no arguments and returns
 *           nothing; its 0x28-byte frame holds the 0x10-byte incoming argument
 *           area, the 12-byte array at 0x10 and $ra at 0x20.
 *           Shape evidence: the same-target sibling at 0x801F31A0 copies the
 *           same three record words to offsets 0x10/0x14/0x18 of the same
 *           0x28-byte frame and calls the same draw routine at 0x801F31E8, so
 *           its statement order, constant-offset copies and unfilled load
 *           delay slots are reused here with this body's own accumulators and
 *           literals.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 38/38 instructions, 152 -> 152 bytes, exact on the first native
 * gate (52.63% at the seed). The seed bound the post-call cursor read to the
 * same pointer local as the entry read and measured the entry record in a1
 * with the advancing word in v1; the original splits those two reads across
 * v1 (entry) and a0 (post-call), so the fix was a second pointer local for the
 * post-call group. The declaration order of that local measured neutral at the
 * same score, so the split, not the order, is the load-bearing shape.
 */
void advanceWorkOffsetAndVelocityThenDrawQuadAndAdvanceStateByte1PastLimitWorld02Area10013_801F3108(
    void) {
  s32 args[3];
  u8* work;
  u8* tail;
  s32 step;

  work = D_1F800044;
  step = FIELD_REF(s32, work, 0x20) + 0x100000;
  FIELD_REF(s32, work, 0x20) = step;
  FIELD_REF(s32, work, 0x14) += step;
  args[0] = FIELD_REF(s32, work, 0x34);
  args[1] = FIELD_REF(s32, work, 0x38);
  args[2] = FIELD_REF(s32, work, 0x3C);
  func_801F31E8(args, FIELD_REF(s32, work, 0x14));
  tail = D_1F800044;
  if (FIELD_REF(s32, tail, 0x14) > 0x7FFFFFF) {
    tail[1]++;
  }
}
