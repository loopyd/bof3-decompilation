#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1E74
 * @behavior dispatches through the local offset jump table progressHandlerTable3,
 *           indexed by the signed byte fairyProgress[0].
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchProgressTable3(void)
{
  progressHandlerTable3[(s8)fairyProgress[0]]();
}
