#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2610
 * @behavior increments D_80148652.
 * @status exact
 * @match 100.00
 */
void advanceUiSubstep(void) {
  u8 *cell = &D_80148652;
  *cell += 1;
}
