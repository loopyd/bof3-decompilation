#include "bof3/battle/battle15_internal.h"

/* @source 0x8009AF7C
 * Byte-identical duplicate of selectionSetupEnabled; its callers branch on the
 * result before storing either the plain slot byte or the value derived from it.
 * @behavior UNKNOWN: exact behavior is not yet documented.
 * @status exact
 * @match 100.00
 * @residual none
 */

u8 selectionSetupEnabledB(void) {
  return 1u;
}
