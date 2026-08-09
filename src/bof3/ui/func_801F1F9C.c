#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1F9C
 * @behavior dispatches through the local offset jump table progressHandlerTable4,
 *           indexed by the signed byte fairyProgress[0].
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801F1F9C(void)
{
  progressHandlerTable4[(s8)fairyProgress[0]]();
}
