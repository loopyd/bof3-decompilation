#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800AC7D0
 * @behavior Scans the thirty 0x98-byte records based at D_8014688E for the
 * first whose leading byte is 7 and whose byte +0x8C matches id; returns that
 * record index, or 0xFF when no record matches.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the record is modelled as one struct array (single extern
 * symbol, 0x98-byte elements, tested bytes at +0x00 and +0x8C) so each access
 * keeps the symbol's %hi/%lo folded into the load. Declaring the base byte and
 * the +0x8C byte as two separate u8 arrays made the compiler hoist both base
 * addresses into registers (18/30 instructions, 60.00%).
 */
u8 func_800AC7D0(u8 id)
{
  u8 i;

  i = 0;
  while (i < 30) {
    if (D_8014688E[i].kind == 7 && D_8014688E[i].owner == id) {
      return i;
    }
    i++;
  }
  return 0xFF;
}
