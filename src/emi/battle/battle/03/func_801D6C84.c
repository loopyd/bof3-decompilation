#include "internal.h"

/* @source 0x801D6C84
 * @behavior invokes the handler selected by the battle-state byte.
 */
void func_801D6C84(void)
{
    D_801EACF4[BATTLE_GLOBAL_BYTE_62E2]();
}
