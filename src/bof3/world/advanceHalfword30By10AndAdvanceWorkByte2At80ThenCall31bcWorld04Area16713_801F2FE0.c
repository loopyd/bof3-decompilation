#include "bof3/bof3.h"

/* @source 0x801F31BC @kind function */
extern void func_801F31BC(void);

/* @source 0x801F2FE0
 * @behavior Word 2 of all three state-byte-2 handler runs this payload carries,
 *           the pointer runs at 0x801F4A90, 0x801F4AA8 and 0x801F4AB8. Reads
 *           the scratchpad work record through the pointer cell 0x1F800044
 *           once and keeps the pointer for the whole run, adds 0x10 to record
 *           halfword +0x30, and advances dispatch byte +0x02 when the updated
 *           halfword, sign-extended, is at least 0x80, which sends the next
 *           dispatch of the same run past this word. Calls the payload render
 *           helper 0x801F31BC on every path. Takes no arguments and returns
 *           nothing, and the 0x18-byte frame only keeps $ra.
 *
 * Name evidence: the sibling payload area168/13 carries this same body at
 * 0x801F3738, where it is the exact lift
 * advanceHalfword30By10AndAdvanceWorkByte2At80ThenCall3914World04Area16813_801F3738,
 * and whole-body comparison of the two spans differing only in their call word
 * proves the shared statement order, the +0x10 step, the 0x80 test on the
 * re-read halfword and the dispatch byte increment before that call. That
 * sibling names the callee after its own local helper because it is the one
 * word the two spans do not share; this span passes the local render helper
 * 0x801F31BC instead, which the exact siblings 0x801F303C and 0x801F36FC of
 * this target also call last. The 0x30 halfword step and the 0x80 threshold are
 * established by the bytes alone, so the name states them rather than claiming
 * a game meaning for either.
 *
 * Shape evidence: no byte-identical twin exists. A scan of the first 0x48 bytes
 * of this span across all 398 payloads under out/binaries returns this span
 * alone, and the spans that share its first 0x40 bytes differ at the call word
 * or later: area168/13 0xB38 and area174/13 0xB7C differ in that word alone,
 * while world03/area148/13 0x3A4 and world03/area147/13 0x334 continue past
 * 0x44 with a different tail. The clone lever is therefore declined and the
 * body is written from the original bytes with the seed above.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By10AndAdvanceWorkByte2At80ThenCall31bcWorld04Area16713_801F2FE0(
    void) {
  u8* work;
  s32 offset;

  work = *(u8**)SPAD_ADDR(u8, 0x44u);
  offset = FIELD_REF(u16, work, 0x30) + 0x10;
  FIELD_REF(u16, work, 0x30) = offset;
  if ((s16)offset >= 0x80) {
    work[2]++;
  }
  func_801F31BC();
}
