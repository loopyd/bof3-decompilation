#include "internal.h"

/* @source 0x801D6EEC
 * @behavior dispatches the byte-selected battle handler.
 */
void battle03_dispatch_state_table_ad0c(void) {
  D_801EAD0C[BATTLE_GLOBAL_BYTE_62E2]();
}
