#include "bof3/ui/shop00_internal.h"

/* @source 0x801D66AC
 * @behavior sets D_80148650 to 1 and D_80148651 to 0.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void setUiFlags(void) {
  D_80148650 = 1;
  D_80148651 = 0;
}
