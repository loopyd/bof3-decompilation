#include "bof3/bof3.h"

/* @source 0x801F3A60
 * @behavior Queues the frontend cue 0x3D with level 0x64 and shape 8 through
 *           the shared front cue helper func_80161C20 and returns. The
 *           overlay's own code-pointer run at 0x801F413C reaches this body as
 *           its index 11 step handler. Takes no arguments, reads no state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueFrontendCue3DWorld02Area09213_801F3A60(void) {
  func_80161C20(0x3D, 0x64, 8);
}
