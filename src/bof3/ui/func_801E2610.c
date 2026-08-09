#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2610
 * @behavior increments D_80148652.
 * @status partial
 * @match 37.50
 * @residual non-exact live audit: 3/7 instructions; 28 original bytes versus 32 current.
 */
void func_801E2610(void) {
  D_80148652 += 1;
}
