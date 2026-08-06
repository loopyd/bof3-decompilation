#include "internal.h"

/* @source 0x801F16BC
 * @behavior dispatches through the local six-entry jump table
 *           progressHandlerTable, indexed by the signed byte fairyProgress[0].
 */
void dispatchProgressTable(void)
{
  progressHandlerTable[(s8)fairyProgress[0]]();
}
