#include "bof3/bof3.h"

void func_801AFE18(void* arg0); /* @source 0x801AFE18 */
s32 func_801783C8(s32 arg0);   /* @source 0x801783C8 */
s32 func_801782FC(s32 arg0);   /* @source 0x801782FC */
void func_801F3560(u8* record); /* @source 0x801F3560 */

/* Scratch work pointer cell: the original materialises the cell address and
 * loads the published pointer once for the state byte read below. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* The overlay's own record-table cursor: the original reloads the cell after
 * every call below, and this target's scan func_801F3800 and reset
 * func_801F385C reference the same cell for the same 0x18-byte-stride table. */
extern u8* D_801F4A88; /* @source 0x801F4A88 @kind data */

/* @source 0x801F38B0
 * @behavior Per-frame pass over the 0x20 records of the 0x18-byte-stride work
 *           table based at 0x800E4800, walked through this overlay's own
 *           published cursor cell 0x801F4A88: it fills the 0x20-byte
 *           projection scratch through func_801AFE18, points the cursor at the
 *           table base and sweeps it with an unsigned byte counter. Every
 *           record whose claim byte at offset 0 reads non-zero is advanced:
 *           when the work object published at the scratchpad pointer cell
 *           0x1F800044 has a zero state byte at +1 the record's word at +0x04
 *           advances by 0x2000, its halfword at +0x14 grows by 8 and its byte
 *           at +3 drops by 2; otherwise the word at +0x04 advances by 0x2000
 *           and then by the angle helper func_801783C8 result of its byte at
 *           +0x17 shifted left 8, the word at +0x0C advances by the second
 *           angle helper func_801782FC result of the same shifted byte, itself
 *           shifted left 8, and the byte at +3 again drops by 2. The countdown
 *           byte at +2 then counts down and releases the claim byte at 0 when
 *           it reaches zero, the record is drawn through func_801F3560, and one
 *           is latched as the result. The cursor advances by 0x18 per record;
 *           the pass returns one when any record was advanced or drawn and zero
 *           when the whole table was unclaimed, which is how its in-payload
 *           callers func_801F2E88, 0x801F2F20 and 0x801F3048 test it.
 *
 * Name evidence: this target's exact seeder
 * seedWorkRecordDefaultsAndWorkPositionWorld03Area13213_801F37A4 documents
 * this address as the per-record update of the same 0x18-byte-stride and
 * 0x20-entry table, reading the byte at +1 as the state that selects the two
 * update arms, decrementing the countdown at +2 to release the claim byte at
 * 0, stepping the byte at +3 by -2 and the halfword at +0x14 by +8 and
 * accumulating the words at +0x04 and +0x0C. The three in-payload callers all
 * discard the incoming argument slots, mask the returned byte with 0xFF and
 * use it to decide whether to run their frame work.
 *
 * Shape evidence: the per-frame table pass that fills the same projection
 * scratch, returns a latched active flag and walks the table at 0x800E4800
 * with an unsigned byte counter is this repository's lifted family, e.g.
 * tickAndRenderActiveWorkRecordsScenarioScena0800_801F8BF8; that pass has its
 * own stride, count and per-record handler, so it is shape evidence only and
 * the strides, offsets, constants and helper bindings here come from this
 * function's own original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 tickAndDrawActiveWorkRecordsWorld03Area13213_801F38B0(void) {
  u8 scratch[0x20];
  u8 index;
  u8 countdown;
  u32 active;

  func_801AFE18(scratch);
  active = 0;
  D_801F4A88 = PSX_PTR(u8, 0x800E4800u);
  index = 0u;
  do {
    if (D_801F4A88[0] != 0) {
      if (D_1F800044[1] == 0) {
        *(s32*)(D_801F4A88 + 0x4) += 0x2000;
        *(u16*)(D_801F4A88 + 0x14) += 8;
        D_801F4A88[3] -= 2;
      } else {
        *(s32*)(D_801F4A88 + 0x4) += 0x2000;
        *(s32*)(D_801F4A88 + 0x4) += func_801783C8(D_801F4A88[0x17] << 8);
        *(s32*)(D_801F4A88 + 0xC) += func_801782FC(D_801F4A88[0x17] << 8) << 8;
        D_801F4A88[3] -= 2;
      }
      countdown = D_801F4A88[2] - 1;
      D_801F4A88[2] = countdown;
      if (countdown == 0) {
        D_801F4A88[0] = 0;
      }
      func_801F3560(D_801F4A88);
      active = 1;
    }
    D_801F4A88 += 0x18;
    index += 1u;
  } while (index < 0x20u);

  return active;
}
