#include "internal.h"

/* @source 0x801D67B0
 * @behavior dispatches the byte-selected battle handler.
 */
void battle03_dispatch_state_table_ace8(void) {
  D_801EACE8[BATTLE_GLOBAL_BYTE_62E2]();
}
