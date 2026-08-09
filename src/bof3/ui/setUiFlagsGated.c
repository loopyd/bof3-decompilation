#include "bof3/ui/shop00_internal.h"

/* @source 0x801D66C8
 * @behavior conditionally sets D_80148650 to 1 and D_80148651 to 0 when bit 1 of
 *         D_801490A4 is set.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void setUiFlagsGated(void) {
  if (D_801490A4 & 2) {
    D_80148650 = 1;
    D_80148651 = 0;
  }
}
