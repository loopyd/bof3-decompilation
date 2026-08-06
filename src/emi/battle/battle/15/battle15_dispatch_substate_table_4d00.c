#include "internal.h"

/* @source 0x800A52C4
 * @behavior dispatches the byte-selected battle handler from D_800B4D00.
 */
void battle15_dispatch_substate_table_4d00(void) {
  D_800B4D00[D_801462E4]();
}
