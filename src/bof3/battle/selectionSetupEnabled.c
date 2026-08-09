#include "bof3/battle/battle15_internal.h"

/* @behavior returns the enabled predicate for the following battle selection setup.
 * @source 0x80097EB8
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 selectionSetupEnabled(void) {
  return 1u;
}
