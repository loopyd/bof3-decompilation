#include "bof3/ui/commu00_internal.h"

/* @source 0x801F222C
 * @behavior While the shared frontend mode byte is not 2, sets the low three
 * bits of the shared front/world flag word, starts the 0x25D EMI slot stream,
 * and advances the fairy progress byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F222C(void) {
  if (D_80143BB0 == 2) {
    return;
  }

  D_8014625A |= 7;
  func_80161FDC(0x25Du);
  fairyProgress[0] += 1;
}
