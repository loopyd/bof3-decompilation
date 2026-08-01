#include "internal.h"

/* @source 0x801F16BC
 * @behavior dispatches through the local six-entry jump table
 *           D_801F25EC, indexed by the signed byte D_801448EC[0].
 */
void func_801F16BC(void)
{
  D_801F25EC[(s8)D_801448EC[0]]();
}
