#include "bof3/battle/battle03_internal.h"

/* @source 0x801D6DE4
 * @behavior dispatches the byte-selected battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchStateTableAd00(void)
{
  D_801EAD00[BATTLE_GLOBAL_BYTE_62E2]();
}
