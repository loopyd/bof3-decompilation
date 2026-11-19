#include "bof3/bof3.h"

/* Shared fixed-point X channel that the scratch work record mirrors; the
 * original takes it with a single `lw` and compares it with `slt`, so the cell
 * keeps its 32-bit signed width.
 * @source 0x80149308 @kind unknown */
extern s32 D_80149308;

/* @source 0x801F3348
 * @behavior Loads the shared X channel D_80149308 and, while it is at most
 *   0x2E0000, stores it into the X field at offset 0x34 of the work record
 *   reached through the pointer slot at 0x1F800044. No calls, no frame and no
 *   stack use; the only memory events are the `lw` of the channel, the `lw` of
 *   the scratch pointer slot and the single `sw` into the record, which the
 *   `bnez` on the signed `slt` of 0x2E0000 against the channel skips.
 *
 * Name evidence: the 48-byte span is byte-identical to five exact siblings at
 *   0x801F3348 - emi/world04/area176/13, area177/13, area181/13, area182/13 and
 *   area185/13 - proven by one `cmp` of the payload spans 0x748..0x777; all six
 *   overlays load at 0x801F2C00. Those bodies supplied this statement order,
 *   operand forms and declaration style. The name follows their own spelling:
 *   0x80149308 is the shared fixed-point channel word whose pair partner
 *   0x8014930C the shared map binds, and offset 0x34 of the work area is
 *   GameWorkArea.coord_x_34 in include/bof3/ui/game00_internal.h, read here at
 *   its own anchored offset rather than through that header.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copySharedXToWorkXIfInRangeWorld04Area18013_801F3348(void) {
  s32 value;
  u8* work;

  value = D_80149308;
  if (value <= 0x2E0000) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    *(s32*)(work + 0x34) = value;
  }
}
