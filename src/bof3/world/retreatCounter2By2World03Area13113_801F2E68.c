#include "bof3/world/area13113_internal.h"

/* @source 0x801F2E68
 * @behavior Overlay counter step: reads the shared halfword counter2 at
 *           0x8014932A, stores the constant 2 into the area status byte at
 *           0x80149333, then writes counter2 back retreated by 0x02. The body is
 *           the retreat-direction member of the pair that surrounds it: its
 *           0x28 bytes are identical to advanceCounter2By2World03Area13113_801F2E90
 *           except that the `addiu` at 0x801F2E7C is -0x02 instead of +0x02, and
 *           it equals the target's retreatCounter2By10World03Area13113_801F2D78
 *           word for word except that immediate, so the read-first order, the
 *           two cells and the declaration style come from those same-target
 *           siblings while the step size comes from this span's own bytes. The
 *           function has no `jal` caller in the payload: its only in-image
 *           reference is the word at 0x801F3CA4 of the code-pointer run at
 *           0x801F3C88, where it sits between 0x801F2E40 and 0x801F2E90, so it is
 *           a dispatch entry and its prologue-less leaf shape (no frame, no
 *           saved register, `jr $ra` with a `nop` delay slot) is the entry shape,
 *           not a fragment. The span is byte-identical to the already exact
 *           lifts at 0x801F41E0, 0x801F3148 and 0x801F4908 of the sibling
 *           overlays, which supply the shape only; each keeps its own address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter2By2World03Area13113_801F2E68(void) {
  u16 count_;

  count_ = counter2;
  D_80149333 = 2;
  counter2 = (u16)(count_ - 0x02);
}
