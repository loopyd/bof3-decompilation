#include "internal.h"

/* @source 0x801F18BC
 * @behavior dispatches through the local six-entry offset jump table
 *           D_801F25EC, indexed by the signed byte D_801448EC[0].
 */
void func_801F18BC(void)
{
  D_801F25EC[(s8)D_801448EC[0] + 6]();
}
