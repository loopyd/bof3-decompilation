#include "internal.h"

/* @source 0x801F1F10
 * @behavior dispatches through the local offset jump table D_801F268C,
 *           indexed by the unsigned byte D_801448ED.
 */
void func_801F1F10(void)
{
  D_801F268C[D_801448ED]();
}
