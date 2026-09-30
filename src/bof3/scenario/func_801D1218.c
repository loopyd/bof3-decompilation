#include "bof3/scenario/sce10eff_internal.h"

/* @behavior dispatches byte 3 of the scratchpad-resident state object through
 * the local handler table at 0x801D2724, projects the scratch position, and,
 * while that byte stays non-zero, rebuilds the scene transform, submits a local
 * primitive batch and restores the matrix stack.
 * @source 0x801D1218
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D1218(void) {
  u8* scratch;
  u8  handler_index;
  u32 slot_offset;

  slot_offset = 0x44u;
  scratch = PSX_REF(u8*, SPAD_BASE + slot_offset);
  handler_index = scratch[3];
  D_801D2724[handler_index]();

  func_801D2658();

  scratch = PSX_REF(u8*, SPAD_BASE + slot_offset);
  if (scratch[3] != 0u) {
    setupSceneObjectTransform();
    func_801D165C(0x20, 0x50, 0x1F);
    emitRadialTranslucentQuads();
    PopMatrix();
  }
}
