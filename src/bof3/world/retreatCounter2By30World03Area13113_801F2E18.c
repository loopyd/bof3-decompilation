#include "bof3/world/area13113_internal.h"

/* @source 0x801F2E18
 * @behavior Overlay counter step: reads the shared halfword counter2 at
 *           0x8014932A, stores the constant 2 into the area status byte at
 *           0x80149333, then writes counter2 back retreated by 0x1E. The
 *           declare-only header include/bof3/world/area13113_internal.h names
 *           both cells. The decoded bytes at 0x801F2E2C are
 *           `addiu $v0, $v0, -0x1E`, so the step is -0x1E and the decimal
 *           counter-step naming family (retreatCounter2By10/By15/By20) makes it
 *           By30. The body equals the same-target advance sibling at
 *           0x801F2E40 word for word except that one immediate (0x1E versus
 *           -0x1E), and the retreat-direction sibling at 0x801F2D78 supplies the
 *           subtract form. The instruction at 0x801F2E18 has no `jal` caller;
 *           its only in-image reference is the word at 0x801F3C9C, a slot of the
 *           code-pointer run based at 0x801F3C48 that lists the neighbouring
 *           counter steps in address order, so it is a dispatch entry whose
 *           prologue-less leaf shape (no frame, no saved register, `jr $ra` with
 *           a `nop` delay slot) is the entry shape, not a fragment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter2By30World03Area13113_801F2E18(void) {
  u16 count_;

  count_ = counter2;
  D_80149333 = 2;
  counter2 = (u16)(count_ - 0x1E);
}
