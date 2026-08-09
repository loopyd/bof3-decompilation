#include "bof3/scenario/scena16_internal.h"

/* @behavior returns immediately.
 * @source 0x801F8530
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void noopRecordHandler(void) {
  return;
}
