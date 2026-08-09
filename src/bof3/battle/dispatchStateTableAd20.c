#include "bof3/battle/battle03_internal.h"

/* @source 0x801D71A4
 * @behavior dispatches the byte-selected battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchStateTableAd20(void) {
  D_801EAD20[BATTLE_GLOBAL_BYTE_62E2]();
}
