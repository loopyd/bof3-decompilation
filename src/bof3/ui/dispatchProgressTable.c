#include "bof3/ui/commu00_internal.h"

/* @source 0x801F16BC
 * @behavior dispatches through the local six-entry jump table
 *           progressHandlerTable, indexed by the signed byte fairyProgress[0].
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchProgressTable(void)
{
  progressHandlerTable[(s8)fairyProgress[0]]();
}
