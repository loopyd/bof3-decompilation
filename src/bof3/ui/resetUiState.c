#include "bof3/ui/shop00_internal.h"

/* @source 0x801E262C
 * @behavior sets D_80148650 to 1, and D_80148651/D_80148652 to 0.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void resetUiState(void) {
  D_80148650 = 1;
  D_80148651 = 0;
  D_80148652 = 0;
}
