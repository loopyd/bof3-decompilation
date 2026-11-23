#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell this overlay publishes. The original
 * materializes the cell with `lui` plus an offset load for every access and
 * keeps no saved register for it across the draw call, so the body spells the
 * cell itself at each statement.
 */
extern u8* g_world02Area11113Work; /* @source 0x1F800044 @kind data */

/* Overlay renderer: builds a run of textured, semi-transparent quads through
 * GetTPage, SetDrawMode, SetPolyG4 and SetSemiTrans for the parts block at its
 * first argument, placed at the signed pair it takes second and third. This
 * payload only calls it, so the roles come from this call site. */
void func_801F2DA0(u8* parts, s16 x, s16 y);

/* @source 0x801F2CFC
 * @behavior State handler 1 of the byte-1 dispatch table 0x801F4890 for the
 *           work record published at the scratchpad pointer cell 0x1F800044.
 *           It draws the record's parts block at offset 0x34 through the
 *           overlay renderer 0x801F2DA0 at the signed halfword pair 0x2E and
 *           0x30, then advances both of those unsigned halfwords by 0x40 and
 *           decrements the countdown byte at offset 0x09. Once that countdown
 *           reaches zero it increments the state byte at offset 0x01, which
 *           the dispatcher at 0x801F2C64 reads on the next tick. Takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps $ra
 *           at 0x10 across the draw call.
 *
 * Name evidence: the handler index comes from the in-payload pointer at
 * 0x801F4894, slot 1 of the byte-1 table 0x801F4890 whose entries 0, 1, 2 and
 * 3 are 0x801F2CA8, this body, 0x801F2D80 and 0x801F3130; the renderer role
 * from the GetTPage/SetDrawMode/SetPolyG4/SetSemiTrans body at 0x801F2DA0;
 * the signed pair and the parts block from the `lh` loads at 0x2E and 0x30
 * and the `addiu` of 0x34 passed as its first argument; and the 0x40 step,
 * the countdown byte 0x09 and the state byte 0x01 from the original `lhu`,
 * `addiu 0x40` and `sb`/`lbu` sequences.
 *
 * Shape: written from the original bytes. A payload scan of every file under
 * out/binaries for this 132-byte span returned only this payload's own copy,
 * so no byte-identical sibling exists and the clone and carve levers do not
 * apply; the range carries its own prologue and `jr $ra`. The first gate
 * matched 33/33 instructions and 132 bytes, so no repair rung was owed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderPartsCountdownExpiry(
    void) {
  func_801F2DA0(g_world02Area11113Work + 0x34,
                *(s16*)(g_world02Area11113Work + 0x2E),
                *(s16*)(g_world02Area11113Work + 0x30));
  *(u16*)(g_world02Area11113Work + 0x2E) += 0x40;
  *(u16*)(g_world02Area11113Work + 0x30) += 0x40;
  if (--g_world02Area11113Work[9] == 0) {
    g_world02Area11113Work[1] += 1;
  }
}
