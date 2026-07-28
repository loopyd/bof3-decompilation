#include "internal.h"

/* @source 0x801F1684
 * @behavior clears the three frontend UI selection-state bytes.
 */
void func_801F1684(void)
{
  func_8015C058();
  D_801448EB = 0;
  D_801448EC[0] = 0;
  D_801448ED = 0;
}
