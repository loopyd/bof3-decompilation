#include "bof3/ui/game00_internal.h"

/* @behavior runs the shared world-front update.
 * @source 0x801981B4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void updateWorld(void) {
  func_80198F1C();
}
