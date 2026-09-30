#include "bof3/ui/commu00_internal.h"

/* @source 0x801F22C8
 * @behavior Requests UI mode 6 while the notification gate byte, the pending
 * queue count, and the queue append index are all clear; otherwise sets the low
 * three bits of the shared front/world flag word, starts the 0x25E EMI slot
 * stream, and advances the fairy progress byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F22C8(void) {
  if (D_80145E44 == 0 && COMMU00_PENDING_QUEUE_COUNT == 0 && D_80145E5D == 0) {
    uiMode = 6;
  } else {
    D_8014625A |= 7;
    func_80161FDC(0x25Eu);
    fairyProgress[0] += 1;
  }
}
