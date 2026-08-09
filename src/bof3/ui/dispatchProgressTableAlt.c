#include "bof3/ui/commu00_internal.h"

/* @source 0x801F18BC
 * @behavior dispatches through the local six-entry offset jump table
 *           progressHandlerTable, indexed by the signed byte fairyProgress[0].
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchProgressTableAlt(void)
{
  progressHandlerTable[(s8)fairyProgress[0] + 6]();
}
