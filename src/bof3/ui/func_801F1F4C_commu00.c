#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1F4C
 * @behavior Clears the low three bits of the shared front/world flag word,
 * clears the UI selection state, and dispatches the current task variant
 * resource, once the EMI loader reports the streamed slot ready.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F1F4C(void) {
  if (func_80162D00() == 0) {
    return;
  }

  D_8014625A &= 0xFFF8u;
  clearUiSelectionState();
  dispatchCurrentTaskVariantResource();
}
