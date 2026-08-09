#include "bof3/battle/battle03_internal.h"

/* @source 0x801D6C84
 * @behavior invokes the handler selected by the battle-state byte.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchStateTableAcf4(void)
{
    D_801EACF4[BATTLE_GLOBAL_BYTE_62E2]();
}
