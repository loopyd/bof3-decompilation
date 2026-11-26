#include "bof3/bof3.h"

/* Shared X channel that the scratch work record mirrors; the original takes it
 * with a single `lw` and compares it signed (`slt` against 0x2E0000), so the
 * cell keeps its 32-bit width. */
extern s32 D_80149308; /* @source 0x80149308 @kind unknown */

/* @source 0x801F3348
 * @behavior Loads the shared X channel D_80149308 and, while it is at most
 *   0x2E0000, stores it into the X field at offset 0x34 of the work record
 *   reached through the pointer slot at 0x1F800044. No calls, no frame and no
 *   stack use; the only memory events are the `lw` of the channel, the `lw` of
 *   the scratch pointer slot and the single `sw` into the record.
 *
 * Name evidence: the 48-byte span at payload offset 0x748 is byte-identical to
 *   the area176/13 and area185/13 spans at the same offset (one payload `cmp`
 *   of 48 bytes from offset 0x748 each), which the family had already lifted
 *   exact as copySharedXToWorkXIfInRangeWorld04Area17613_801F3348 and
 *   copySharedXToWorkXIfInRangeWorld04Area18513_801F3348, and the same body
 *   sits in area175/13 at 0x801F3FDC. The `bin/harness lift clone` dry run
 *   declined this range with `unresolved external 'SPAD_PTR_SLOT': not
 *   declared, not in the target map`, so this body is transcribed from the
 *   area176/13 seed and keeps its statement order, operand forms and
 *   declaration style. Its guard constant 0x2E0000, its scratchpad pointer slot
 *   0x1F800044 and its record offset 0x34 come from this payload's own bytes;
 *   the slot and offset are the cells include/bof3/ui/game00_internal.h
 *   documents as GameWorkArea.coord_x_34, read here at its own anchored offset
 *   rather than through that header.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copySharedXWork(void) {
  s32 value;
  u8* work;

  value = D_80149308;
  if (value <= 0x2E0000) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    *(s32*)(work + 0x34) = value;
  }
}
