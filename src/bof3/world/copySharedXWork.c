#include "bof3/bof3.h"

/* Shared X channel that the scratch work record mirrors; the original takes
 * it with a single `lw` and compares it signed, so the cell keeps its 32-bit
 * width and the comparison keeps `slt`.
 * @source 0x80149308 @kind unknown */
extern s32 D_80149308;

/* @source 0x801F3348
 * @behavior Loads the shared X channel D_80149308 and, while it is at most
 *   0x2E0000, stores it into the X field at offset 0x34 of the work record
 *   reached through the pointer slot at 0x1F800044. No calls, no frame and no
 *   stack use; the only memory events are the `lw` of the channel, the `lw` of
 *   the scratch pointer slot and the single `sw` into the record.
 *
 * Name evidence: the 48-byte range is byte-identical to the area175 body at
 *   0x801F3FDC (one cmp of the two payload spans, 0x748..0x778 here against
 *   0x13DC..0x140C there), which is the same read-guard-store of the shared X
 *   channel into the work record's X field; that body supplied this
 *   statement order, operand forms and declaration style. The
 *   `bin/harness lift clone` adapter declined this range with
 *   `unresolved external 'SPAD_PTR_SLOT': not declared, not in the target
 *   map`, so the seed was transcribed by hand from that span. The scratch
 *   pointer slot 0x1F800044 and the mirrored offset 0x34 are the ones
 *   include/bof3/ui/game00_internal.h documents as GameWorkArea.coord_x_34,
 *   read here at its own anchored offset rather than through that header.
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
