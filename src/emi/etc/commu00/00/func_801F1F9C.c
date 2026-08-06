#include "internal.h"

/* @source 0x801F1F9C
 * @behavior dispatches through the local offset jump table progressHandlerTable4,
 *           indexed by the signed byte fairyProgress[0].
 */
void func_801F1F9C(void)
{
  progressHandlerTable4[(s8)fairyProgress[0]]();
}
