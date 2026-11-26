#include "bof3/bof3.h"

/* Shared X channel mirrored into the scratch work record; the original reads
 * it with one `lw` and compares it signed, so the cell keeps its 32-bit width
 * and the comparison keeps `slt`.
 * @source 0x80149308 @kind unknown */
extern s32 D_80149308;

/* @source 0x801F3348
 * @behavior Loads the shared X channel D_80149308 and, while it is at most
 *   0x2E0000, stores it into the X field at offset 0x34 of the work record
 *   reached through the scratchpad pointer slot at 0x1F800044. No calls, no
 *   frame and no stack use; the memory events are the `lw` of the channel, the
 *   `lw` of the scratch pointer slot and the single `sw` into the record.
 *
 * Name evidence: the 48-byte span 0x748..0x778 is byte-identical to the
 *   already-lifted bodies at the same address in emi/world04/area176 and
 *   emi/world04/area185 (one cmp per target over the 0x30-byte payload range),
 *   which are the same read-guard-store of the shared X channel into the work
 *   record's X field; those bodies supplied this statement order, operand
 *   forms and declaration style. `bin/harness lift clone` declined the range
 *   with `unresolved external 'SPAD_PTR_SLOT': not declared, not in the
 *   target map`, so the seed was transcribed by hand. The scratch slot
 *   0x1F800044 and the mirrored offset 0x34 are the ones
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
