#include "internal.h"

/* @source 0x801D6DE4
 * @behavior dispatches the byte-selected battle handler.
 */
void battle03_dispatch_state_table_ad00(void)
{
  D_801EAD00[BATTLE_GLOBAL_BYTE_62E2]();
}
