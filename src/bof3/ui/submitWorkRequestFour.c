#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801C0D34
 * @behavior Calls commitPendingWorkRequest with mode 4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitWorkRequestFour(void) {
  commitPendingWorkRequest(4);
}
