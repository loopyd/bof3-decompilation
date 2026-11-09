#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original materialises it with one
 * lui plus offset load for the opening update, and re-reads it through its own
 * address load after the distance call for the comparison, the release stores
 * and the branch target check.
 * @source 0x1F800044 @kind data
 */
extern u8 *D_1F800044;
/* Pointer cell of the shared work record whose halfword 0x0A this step consumes
 * and whose byte 0x01 it seeds when the scratch work record is released.
 * @source 0x80148210 @kind unknown
 */
extern u16 *D_80148210;

/* Shared resident mover at 0x801B0DFC: it advances the work record by the step
 * word this step leaves in the argument register and consumes no result. */
void func_801B0DFC(void);
/* Shared resident distance routine: takes the coordinate pair and returns its
 * distance, which the callers narrow to a signed halfword. */
s32 func_80154F28(s32 x, s32 y);
/* Shared resident resource request at 0x8014D6B8: takes the resource id and
 * consumes no result. */
void func_8014D6B8(u32 resource);

/* @source 0x801F3F18
 * @behavior Overlay per-frame step of the scratch work record published through
 * the scratchpad pointer cell 0x1F800044. It adds the record's 0x20 step word to
 * its 0x14 motion word, keeps that step word as the argument of the shared mover
 * func_801B0DFC called immediately after, and then queries the shared distance
 * routine func_80154F28 with the record's 0x34/0x38 coordinate pair, narrowing
 * the result to a signed halfword before comparing it against the record's
 * signed halfword at 0x3E. When that halfword is below the narrowed distance the
 * record is released: the raw result is stored into the 0x3E halfword, the 0x14
 * and 0x20 words are zeroed, 2 is written into the handler byte at 0x04,
 * resource 0x39 is requested through func_8014D6B8 and byte 0x01 of the shared
 * record at 0x80148210 is set to 4. Otherwise it consumes two units of that
 * shared record's halfword at 0x0A. Both arms continue into the shared closing
 * frame restore, which no arm writes. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
/*
 * The opening motion-word update reads the cell twice in one expression, so the
 * single load the original keeps is the compiler's own CSE and the step word
 * stays in the register the add leaves it in. The narrowing stays an inline
 * cast at the comparison because the original narrows the copy of the call
 * result that the 0x3E store later uses: a named signed-halfword local let the
 * compiler narrow from the call-result register directly, which cost the sll
 * one differing source register. The release arm copies the cell into a record
 * local because the original re-reads the cell once after the call and stores
 * through that one pointer.
 * The release arm and the shared-halfword decrement are the same statement order
 * as the exact sibling at world04/area172/13 0x801F44C4, which runs the same
 * opening pair, the same distance test against the 0x3E halfword and the same
 * release stores; that sibling adds a flag-bit clear, four marker-byte zeroes
 * and a factor/level tail this span does not have, and narrows into the store
 * register where this span keeps the raw result.
 */
void advanceWorkMotionAndReleaseRecordPast3eElseConsumeSharedBy2World03Area14514_801F3F18(
    void) {
  s32 result;
  u8 *record;

  *(s32 *)(D_1F800044 + 0x14) += *(s32 *)(D_1F800044 + 0x20);
  func_801B0DFC();
  result =
      func_80154F28(*(s32 *)(D_1F800044 + 0x34), *(s32 *)(D_1F800044 + 0x38));
  if (*(s16 *)(D_1F800044 + 0x3e) < (s16)result) {
    record = D_1F800044;
    *(s16 *)(record + 0x3e) = result;
    *(s32 *)(record + 0x14) = 0;
    *(s32 *)(record + 0x20) = 0;
    record[4] = 2;
    func_8014D6B8(0x39);
    ((u8 *)D_80148210)[1] = 4;
  } else {
    D_80148210[5] -= 2;
  }
}
