#include "internal.h"

/* @source 0x801D6C84
 * @behavior invokes the handler selected by the battle-state byte.
 */
void battle03_dispatch_state_table_acf4(void)
{
    D_801EACF4[BATTLE_GLOBAL_BYTE_62E2]();
}
