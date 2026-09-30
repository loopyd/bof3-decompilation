#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1FD8
 * @behavior Sets the low three bits of the shared front/world flag word, starts
 * the 0x25C EMI slot stream, and advances the fairy progress byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F1FD8(void) {
  D_8014625A |= 7;
  func_80161FDC(0x25Cu);
  fairyProgress[0] += 1;
}
