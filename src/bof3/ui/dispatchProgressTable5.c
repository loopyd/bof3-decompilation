#include "bof3/ui/commu00_internal.h"

/* @source 0x801F2020
 * @behavior dispatches through the local offset jump table progressHandlerTable5,
 *           indexed by the signed byte fairyProgress[0].
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressTable5(void)
{
  progressHandlerTable5[(s8)fairyProgress[0]]();
}
