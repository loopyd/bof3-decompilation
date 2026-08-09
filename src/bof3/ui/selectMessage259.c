#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1770
 * @behavior selects message 0x259 and advances its local state byte
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void selectMessage259(void) {
  func_80161FDC(0x259u);
  fairyProgress[0] += 1;
}
