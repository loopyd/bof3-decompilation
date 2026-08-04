#include "internal.h"

/* @source 0x801F1E74
 * @behavior dispatches through the local offset jump table D_801F2678,
 *           indexed by the signed byte D_801448EC[0].
 */
void func_801F1E74(void)
{
  D_801F2678[(s8)D_801448EC[0]]();
}
