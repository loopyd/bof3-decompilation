#include "bof3/ui/game00_internal.h"

/* @behavior resolves the current world state ID through func_8019A194 and
 * tail-calls the matching no-argument handler from the handler table at
 * D_801C85C4.
 * @source 0x801A839C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801A839C(void)
{
  u8 index;

  index = func_8019A194();
  D_801C85C4[index]();
}
