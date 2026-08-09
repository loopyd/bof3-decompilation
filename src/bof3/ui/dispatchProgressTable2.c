#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1B8C
 * @behavior dispatches through the local offset jump table progressHandlerTable2,
 *           indexed by the signed byte fairyProgress[0].
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchProgressTable2(void)
{
  progressHandlerTable2[(s8)fairyProgress[0]]();
}
