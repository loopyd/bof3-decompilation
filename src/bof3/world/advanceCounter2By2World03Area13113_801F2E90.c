#include "bof3/world/area13113_internal.h"

/* @source 0x801F2E90
 * @behavior Overlay counter step: reads the shared halfword counter2 at
 *           0x8014932A, stores the constant 2 into the area status byte at
 *           0x80149333, then writes counter2 back advanced by 0x02. The
 *           read-first order and the two cells are the same pair the adjacent
 *           retreatCounter2By10World03Area13113_801F2D78 uses at 0x801F2D78
 *           with the immediate negated (-0x0A), and its declare-only header
 *           include/bof3/world/area13113_internal.h names both cells, so the
 *           immediate and the cell roles come from that same-target sibling.
 *           0x801F2E90 has no `jal` caller in the payload: its only in-image
 *           reference is a word of the code-pointer run based at 0x801F3C48,
 *           where it sits between func_801F2E68 and
 *           copyModeByteWorld03Area13113_801F2EB8, so it is a dispatch entry
 *           and its prologue-less leaf shape (no frame, no saved register,
 *           `jr $ra` with a `nop` delay slot) is the entry shape, not a
 *           fragment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounter2By2World03Area13113_801F2E90(void) {
  u16 count_;

  count_ = counter2;
  D_80149333 = 2;
  counter2 = (u16)(count_ + 0x02);
}
