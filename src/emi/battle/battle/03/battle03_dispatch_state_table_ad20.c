#include "internal.h"

/* @source 0x801D71A4
 * @behavior dispatches the byte-selected battle handler.
 */
void battle03_dispatch_state_table_ad20(void) {
  D_801EAD20[BATTLE_GLOBAL_BYTE_62E2]();
}
