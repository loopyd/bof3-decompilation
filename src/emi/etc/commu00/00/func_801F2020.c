#include "internal.h"

/* @source 0x801F2020
 * @behavior dispatches through the local offset jump table progressHandlerTable5,
 *           indexed by the signed byte fairyProgress[0].
 */
void func_801F2020(void)
{
  progressHandlerTable5[(s8)fairyProgress[0]]();
}
