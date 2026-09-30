#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1AFC
 * @behavior Clears the frontend UI selection state and dispatches the current
 * task variant resource while the shared mode byte is clear.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F1AFC(void) {
  if (D_80143BB0 == 0) {
    clearUiSelectionState();
    dispatchCurrentTaskVariantResource();
  }
}
