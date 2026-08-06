#include "internal.h"

/* @source 0x801F1E74
 * @behavior dispatches through the local offset jump table progressHandlerTable3,
 *           indexed by the signed byte fairyProgress[0].
 */
void dispatchProgressTable3(void)
{
  progressHandlerTable3[(s8)fairyProgress[0]]();
}
