#include "bof3/ui/shop00_internal.h"

/**
 * @source 0x801DEE20
 * @behavior Resets the shop phase state when the global gate is clear.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801DEE20(void) {
  if (D_80143C40 == 0) {
    phaseTimer = 0;
    D_80148650 = 1;
    D_80148651 = 0;
    D_80148652 = 0;
    D_8014865F = 1;
  }
}
