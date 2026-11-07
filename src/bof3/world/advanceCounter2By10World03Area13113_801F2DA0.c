#include "bof3/world/area13113_internal.h"

/* @source 0x801F2DA0
 * @behavior Overlay counter step: reads the shared halfword counter2 at
 *           0x8014932A, stores the constant 2 into the area status byte at
 *           0x80149333, then writes counter2 back advanced by 0x0A. The
 *           declare-only header include/bof3/world/area13113_internal.h names
 *           both cells, and the same-target siblings at 0x801F2D78 (-0x0A) and
 *           0x801F2E90 (+0x02) carry the identical read-first, store-2,
 *           add-immediate, store-back leaf shape, so the immediate sign and the
 *           cell roles come from those siblings. The instruction at 0x801F2DA0
 *           has no `jal` caller; its only in-image reference is a word of the
 *           code-pointer run at 0x801F3C90, so it is a dispatch entry whose
 *           prologue-less leaf shape (no frame, no saved register, `jr $ra` with
 *           a `nop` delay slot) is the entry shape, not a fragment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounter2By10World03Area13113_801F2DA0(void) {
  u16 count_;

  count_ = counter2;
  D_80149333 = 2;
  counter2 = (u16)(count_ + 0x0A);
}
