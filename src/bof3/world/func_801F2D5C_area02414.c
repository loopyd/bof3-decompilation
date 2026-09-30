#include "bof3/world/area02414_internal.h"

/* @behavior overlay frame entry: draws the local spin work, advances the frame
 * state, dispatches every active work-entry state handler, then increments
 * scratch-work byte 0x01 when no active work was processed.
 * @source 0x801F2D5C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2D5C(void) {
  drawSpin();
  func_801F362C();

  if ((dispatchWorkStates() & 0xFF) == 0) {
    D_1F800044[1]++;
  }
}
