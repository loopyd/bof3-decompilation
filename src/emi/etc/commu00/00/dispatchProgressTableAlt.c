#include "internal.h"

/* @source 0x801F18BC
 * @behavior dispatches through the local six-entry offset jump table
 *           progressHandlerTable, indexed by the signed byte fairyProgress[0].
 */
void dispatchProgressTableAlt(void)
{
  progressHandlerTable[(s8)fairyProgress[0] + 6]();
}
