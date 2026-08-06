#include "internal.h"

/* @source 0x801F228C
 * @behavior dispatches through the local offset jump table progressHandlerTable6,
 *           indexed by the signed byte fairyProgress[0].
 */
void func_801F228C(void)
{
  progressHandlerTable6[(s8)fairyProgress[0]]();
}
