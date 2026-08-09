#include "bof3/ui/game00_internal.h"

/* @behavior calls the entry-1 update slice, then finalizes the shared front-end frame.
 * @source 0x801993F0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void finalizeFrame(void) {
  func_801D0D9C();
  func_80158C80();
}
