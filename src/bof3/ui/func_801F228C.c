#include "bof3/ui/commu00_internal.h"

/* @source 0x801F228C
 * @behavior dispatches through the local offset jump table progressHandlerTable6,
 *           indexed by the signed byte fairyProgress[0].
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801F228C(void)
{
  progressHandlerTable6[(s8)fairyProgress[0]]();
}
