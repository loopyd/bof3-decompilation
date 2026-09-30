#include "bof3/battle/battle15_internal.h"

/* @behavior returns the enabled predicate for the following battle selection setup.
 * @source 0x80097EB8
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 selectionSetupEnabled(void) {
  return 1u;
}
