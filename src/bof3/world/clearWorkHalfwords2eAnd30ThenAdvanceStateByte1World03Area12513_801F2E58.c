#include "bof3/bof3.h"

/* @source 0x801F2E58
 * @behavior Entry 6 of this overlay's state-handler run at 0x801F36CC, which
 *           dispatchStateByte1Table36ccWorld03Area12513_801F2C04 selects with
 *           the unsigned work byte at offset +0x1 of the work object published
 *           at the scratchpad pointer cell 0x1F800044 and calls with no
 *           arguments; the run is not bounded, and this entry is the seventh of
 *           its nine words. It loads that pointer cell once and reaches every
 *           field through the single loaded base: it reads the unsigned work
 *           byte at +0x1, clears the halfwords at +0x2E and +0x30, then stores
 *           that byte plus one. The two cleared halfwords are the step state the
 *           following entry 0x801F2E7C advances in the same overlay, and the
 *           byte store moves the next frame's dispatch to that entry. The
 *           pointer-cell load sits before the first field access with an
 *           unfilled delay slot, the function has no prologue and no frame
 *           because it calls nothing, and it reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkHalfwords2eAnd30ThenAdvanceStateByte1World03Area12513_801F2E58(void) {
  u8* work;
  u8 state;

  work = SPAD_PTR_SLOT(u8, 0x44);
  state = work[1];
  *(u16*)(work + 0x2E) = 0;
  *(u16*)(work + 0x30) = 0;
  work[1] = state + 1;
}
