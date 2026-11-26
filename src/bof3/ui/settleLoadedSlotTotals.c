#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1874
 * @behavior Once the EMI loader reports the streamed slot ready, clears the UI
 * selection state, dispatches the current task variant resource, recomputes the
 * state-10 and state-11 progress totals, and runs the record notification
 * advance pass.
 * @status exact
 * @match 100.00
 * @residual none
 */
void settleLoadedSlotTotals(void) {
  if (func_80162D00() != 0) {
    clearUiSelectionState();
    callCurrentTaskVariant();
    accumulateKindWeightTotals();
    func_801EF52C();
  }
}
