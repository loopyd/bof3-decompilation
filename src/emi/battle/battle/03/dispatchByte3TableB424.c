#include "internal.h"

/* @source 0x801E3BD0
 * @behavior dispatches through D_801EB424 using byte three of the object addressed
 * by the non-volatile scratchpad pointer cell battleWork.
 */
void NO_SIBLING_CALLS dispatchByte3TableB424(void) {
  D_801EB424[((Battle03LocalWork*)battleWork)->unk_03]();
}
