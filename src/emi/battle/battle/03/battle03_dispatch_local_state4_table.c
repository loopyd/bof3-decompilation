#include "internal.h"

/* @behavior dispatches the current local state-4 byte through its table.
 * @source 0x801E1298
 */
void NO_SIBLING_CALLS battle03_dispatch_local_state4_table(void) {
  D_801EB210[D_1F800044->unk_04]();
}
