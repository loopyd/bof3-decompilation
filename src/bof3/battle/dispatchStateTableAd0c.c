#include "bof3/battle/battle03_internal.h"

/* @source 0x801D6EEC
 * @behavior dispatches the byte-selected battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchStateTableAd0c(void) {
  D_801EAD0C[BATTLE_GLOBAL_BYTE_62E2]();
}
