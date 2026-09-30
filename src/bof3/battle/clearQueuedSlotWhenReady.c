#include "bof3/battle/battle03_internal.h"

/* @behavior clears the current queued-slot bytes once the shared battle
 * readiness predicate reports ready, otherwise leaves the frame's remaining work
 * to the shared idle helper. Handler index 1 of the two-entry table dispatched by
 * dispatchByte1PairFlagged through scratch work byte +0x01.
 * @source 0x801E68EC
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearQueuedSlotWhenReady(void) {
  if (func_8014D978() != 0u) {
    clearQueuedSlotBytes();
  } else {
    func_8014D4E0();
  }
}
