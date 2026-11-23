#include "bof3/world/area03004_internal.h"

/* @behavior Decrements work countdown byte 9, then reloads the work pointer. A
 * zero reloaded countdown advances handler byte 1. Always calls the
 * bounded work-pointer queue appender.
 * @source 0x801D3510
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementExpiryQueuePointer(void) {
  u8* countdownWork;
  u8* dispatchWork;

  countdownWork = (u8*)D_1F800044;
  countdownWork[9u] = (u8)(countdownWork[9u] - 1u);
  dispatchWork = (u8*)D_1F800044;
  if (dispatchWork[9u] == 0u) {
    dispatchWork[1u] = (u8)(dispatchWork[1u] + 1u);
  }
  queueUnprojectedActiveWork();
}
