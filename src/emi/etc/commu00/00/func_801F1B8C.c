#include "internal.h"

/* @source 0x801F1B8C
 * @behavior dispatches through the local offset jump table D_801F2610,
 *           indexed by the signed byte D_801448EC[0].
 */
void func_801F1B8C(void)
{
  D_801F2610[(s8)D_801448EC[0]]();
}
