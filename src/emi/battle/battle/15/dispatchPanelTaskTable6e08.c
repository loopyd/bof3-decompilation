#include "internal.h"

/* @source 0x800B2088
 * @behavior Dispatches the handler selected by the panel task's state byte.
 */

void dispatchPanelTaskTable6e08(void) {
  D_800B6E08[D_80148648->unk_00[2]]();
}
