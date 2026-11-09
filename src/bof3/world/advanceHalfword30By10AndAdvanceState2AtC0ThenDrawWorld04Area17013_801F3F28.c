#include "bof3/bof3.h"

void drawRouteAxisSplitTexturedQuadPairAtWorkCellScaledByHalfword30World04Area17013_801F40F0(
    void);

/* @source 0x801F3F28
 * @behavior Per-frame handler of the overlay's work record, published at the
 *           scratchpad pointer cell 0x1F800044 and reached through the pointer
 *           run based at 0x801F57BC (this address is its word 0x36, the entry
 *           immediately before the exact work-gate handler at 0x801F3F84), so
 *           it has no jal caller: the dispatcher reaches it. Advances the
 *           record's halfword at 0x30 by 0x10 and stores the truncated value;
 *           once that stored signed halfword reaches 0xC0 it advances the
 *           record's state byte at 0x02 by one. It then calls the overlay
 *           routine at 0x801F40F0, which re-reads the same record, projects
 *           the route halfword (0x3A when the record route byte at 0x08 is
 *           non-zero, otherwise 0x36) into the four scratchpad halfwords at
 *           0x14, 0x1C, 0x24 and 0x2C, and builds the record's sprite
 *           primitive. The cell is loaded once and that one base serves the
 *           halfword arm and the byte store. Takes no arguments and returns
 *           nothing; its 0x18-byte frame only holds $ra at 0x10 across the
 *           call, whose delay slot stays empty.
 *
 * Shape evidence: the near-identical shape sibling
 * advanceHalfword30By20AndAdvanceState2At100World04Area15713_801F3718 carries
 * the same body up to the constants and differs from these bytes only in the
 * step 0x20, the limit 0x100 and the missing tail call, so its statement
 * order, offset-local spelling and halfword read-modify-write form are copied
 * here with this function's own literals and call target from the original
 * bytes. The copies of this body in emi/world04/area169/13 at 0x801F2F0C and
 * in emi/world04/area171/13 at 0x801F2F00 differ in the immediate of their own
 * jal (and area171's delay slot passes a0), so they are relocation-different
 * siblings: shape evidence only, never clone eligibility.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By10AndAdvanceState2AtC0ThenDrawWorld04Area17013_801F3F28(
    void) {
  u8* record;
  s16* half;

  record = SPAD_PTR_SLOT(u8, 0x44u);
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x10);
  if (*half >= 0xC0) {
    record[2]++;
  }
  drawRouteAxisSplitTexturedQuadPairAtWorkCellScaledByHalfword30World04Area17013_801F40F0();
}
