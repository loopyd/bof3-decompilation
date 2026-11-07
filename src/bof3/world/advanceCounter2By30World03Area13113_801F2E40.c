#include "bof3/world/area13113_internal.h"

/* @source 0x801F2E40
 * @behavior Overlay counter step: reads the shared halfword counter2 at
 *           0x8014932A, stores the constant 2 into the area status byte at
 *           0x80149333, then writes counter2 back advanced by 0x1E. The
 *           declare-only header include/bof3/world/area13113_internal.h names
 *           both cells, and the same-target siblings at 0x801F2DA0 (+0x0A) and
 *           0x801F2E90 (+0x02) carry the identical read-first, store-2,
 *           add-immediate, store-back leaf shape, so the immediate sign and the
 *           cell roles come from those siblings; the decoded bytes at
 *           0x801F2E54 are `addiu $v0, $v0, 0x1E`, so the step is +0x1E and the
 *           counter-step naming family (advanceCounter2By10/By20/By4) makes it
 *           By30. The instruction at 0x801F2E40 has no `jal` caller; its only
 *           in-image reference is the word at 0x801F3CA0, a slot of the
 *           code-pointer run based at 0x801F3C48 that lists the neighbouring
 *           counter steps in address order, so it is a dispatch entry whose
 *           prologue-less leaf shape (no frame, no saved register, `jr $ra` with
 *           a `nop` delay slot) is the entry shape, not a fragment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounter2By30World03Area13113_801F2E40(void) {
  u16 count_;

  count_ = counter2;
  D_80149333 = 2;
  counter2 = (u16)(count_ + 0x1E);
}
