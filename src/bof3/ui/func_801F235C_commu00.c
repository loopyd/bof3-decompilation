#include "bof3/ui/commu00_internal.h"

/* @source 0x801F235C
 * @behavior Clears the low three bits of the shared front/world flag word and
 * clears the UI selection state, once the EMI loader reports the streamed slot
 * ready.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F235C(void) {
  if (func_80162D00() == 0) {
    return;
  }

  D_8014625A &= 0xFFF8u;
  clearUiSelectionState();
}
