#include "internal.h"

/* @source 0x800A59AC
 * @behavior dispatches the current battle-selection handler from D_800B4D14.
 */
void dispatchSubstateTable4d14(void) {
  D_800B4D14[D_801462E4]();
}
