#include "internal.h"

/* @source 0x801F1B8C
 * @behavior dispatches through the local offset jump table progressHandlerTable2,
 *           indexed by the signed byte fairyProgress[0].
 */
void dispatchProgressTable2(void)
{
  progressHandlerTable2[(s8)fairyProgress[0]]();
}
