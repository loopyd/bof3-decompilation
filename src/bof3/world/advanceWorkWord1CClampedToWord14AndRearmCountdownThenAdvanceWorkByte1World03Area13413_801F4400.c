#include "bof3/bof3.h"

void func_801F4B6C(void);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer for each of the four access groups below. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F4400
 * @behavior Overlay work-record step handler, entry 1 of the handler table
 * based at 0x801F5828 that this target's exact
 * dispatchStateByte1Table5828World03Area13413_801F4344 indexes with the work
 * record's state byte at +1, reached the frame after its entry 0 at 0x801F4388
 * advances that byte. Entry 0 seeds the fields this body moves: it arms the
 * countdown byte at +0x09 with 0x40, clears the halfword at +0x20, stores
 * 0x01000000 in the word at +0x24, and copies the saved words at +0x34, +0x38
 * and +0x3C into the record's words at +0x0C, +0x10 and +0x14 while also
 * starting the word at +0x1C from that +0x3C copy.
 * This handler then advances the record's signed word at +0x1C by the same
 * 0x01000000 step and clamps it to the word at +0x14 plus 0x08000000 (eight
 * steps beyond it), so the interpolated +0x1C value cannot pass its limit;
 * sweeps the halfword at +0x20 down by 0x200; runs the overlay helper at
 * 0x801F4B6C, which takes no arguments and returns nothing; and decrements the
 * countdown byte at +0x09 modulo 256. When that countdown reads zero it re-arms
 * the byte with 0x10 and advances the record's state byte at +0x01 by one, so
 * the next frame dispatches entry 2 at 0x801F44C0, which continues the same
 * +0x20 sweep while adding -0x00100000 to the +0x24 word and calls the same
 * 0x801F4B6C helper. Takes no arguments and returns nothing; its 0x18-byte
 * frame only holds $ra at 0x10 across the call, whose delay slot keeps the
 * +0x20 halfword store. The roles of the record's words at +0x14 and +0x1C,
 * of its halfword at +0x20 and of the 0x01000000 step stay described by offset.
 *
 * Name evidence: the only in-image reference to 0x801F4400 is word 1 of the
 * handler table based at 0x801F5828 (offset 0x2C2C), the fourth word of the
 * pointer run that starts at 0x801F581C; there is no jal caller, which is the
 * dispatch shape rather than missing evidence. That table is indexed with the
 * work record's state byte at +1 by the exact same-target dispatcher
 * dispatchStateByte1Table5828World03Area13413_801F4344, whose entries are this
 * overlay's step handlers. The advance-and-clamp role comes from this body's
 * own immediates and from entry 0's seeding of the same fields; the countdown
 * re-arm and the state-byte advance come from the byte cells the body writes.
 *
 * Shape evidence: the area103 overlay of world02 holds a copy of these 0xC0
 * bytes at 0x801F37A0 that differs in exactly the two immediate bytes of its
 * jal at 0x801F37F4 (0x801F38DC there, 0x801F4B6C here), so it is a
 * relocation-different sibling: shape evidence only, never clone eligibility.
 * The statement order, the pointer-cell reload points and the u8 accumulator
 * spelling of the countdown group follow this target's exact
 * claimWorkRecordEveryFourthCountdownTickThenAdvanceWorkByte1World03Area13413_801F4288
 * (`countdown = (u8)(work[9] - 1); work[9] = countdown;` tested for zero, with
 * the re-armed byte written through the cell symbol); this function's own
 * literals, clamp limit, step, sweep value and call target come from its
 * original bytes.
 * @status partial
 * @match 95.83
 * @residual The re-arm constant 0x10 is materialised one instruction later
 *           than the original. The original computes it in the delay slot of
 *           the countdown test (li v0,16 at 0x801F4480) and then reloads the
 *           cell into v1 with an unfilled load delay, while this build
 *           reloads the cell into v0 first and materialises the constant at
 *           the store (nop at 0x801F4480, li v0,16 after the load). The other
 *           46 of 48 instructions match, and size, frame and call are
 *           unchanged. Both reachable clean-C spellings were measured: the
 *           cross-block form of the constant makes it a global pseudo that
 *           conflicts with the countdown's own v0 (allocator dump: "78
 *           conflicts: 78 2 29"), and the block-local form keeps its own
 *           definition but is emitted after the cell reload, so the delay slot
 *           stays a nop. The record's word at +0x14, its halfword at +0x20 and
 *           the step 0x01000000 stay described by offset.
 */
void advanceWorkWord1CClampedToWord14AndRearmCountdownThenAdvanceWorkByte1World03Area13413_801F4400(
    void) {
  u8* work;
  u8* sweep;
  u8* cursor;
  s32 next;
  s32 limit;
  u8 countdown;

  work = D_1F800044;
  next = FIELD_REF(s32, work, 0x1C) + 0x1000000;
  limit = FIELD_REF(s32, work, 0x14) + 0x8000000;
  FIELD_REF(s32, work, 0x1C) = next;
  if (limit < next) {
    FIELD_REF(s32, work, 0x1C) = limit;
  }
  sweep = D_1F800044;
  FIELD_REF(u16, sweep, 0x20) -= 0x200;
  func_801F4B6C();
  cursor = D_1F800044;
  countdown = (u8)(cursor[9] - 1);
  cursor[9] = countdown;
  if (countdown == 0) {
    D_1F800044[9] = 0x10;
    D_1F800044[1]++;
  }
}
