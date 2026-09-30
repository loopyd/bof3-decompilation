#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1EB0
 * @behavior Sets the low three bits of the shared front/world flag word, starts
 * the EMI slot stream selected by the fairy slot index (0x25B when that index is
 * 3, otherwise 0x25A), and advances the fairy progress byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F1EB0(void) {
  D_8014625A |= 7;
  if (fairySlotIndex == 3) {
    func_80161FDC(0x25B);
  } else {
    func_80161FDC(0x25A);
  }
  fairyProgress[0] += 1;
}
