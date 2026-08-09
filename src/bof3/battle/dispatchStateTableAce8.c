#include "bof3/battle/battle03_internal.h"

/* @source 0x801D67B0
 * @behavior dispatches the byte-selected battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchStateTableAce8(void) {
  D_801EACE8[BATTLE_GLOBAL_BYTE_62E2]();
}
