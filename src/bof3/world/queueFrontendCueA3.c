#include "bof3/bof3.h"

/* @source 0x801F2EC4
 * @behavior Queues the frontend cue 0xA3 with level 0x64 and shape 8 through
 *   the shared front cue helper func_80161C20 and returns. The helper is
 *   declared by include/bof3/core.h, so this body needs no prototype of its
 *   own. Takes no arguments, reads no state and builds a 0x18-byte frame only
 *   to keep the return address across the call. The payload has no jal to this
 *   address; its only in-image reference is the code-pointer word at
 *   0x801F4BB8 in the trailing pointer run of the same overlay, which starts at
 *   0x801F4B90 with the payload's first function 0x801F2C04.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueFrontendCueA3(void) {
  func_80161C20(0xA3, 0x64, 8);
}
