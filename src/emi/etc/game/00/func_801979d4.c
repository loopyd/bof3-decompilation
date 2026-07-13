#include "internal.h"

/* @behavior advances the front-end into state 2 when the shared input/status
 * flag is set, then runs the common state update.
 * @source 0x801979d4 func_801979d4
 */
void func_801979d4(void) {
  if ((DAT_801490a4 & 2u) != 0u) {
    DAT_80146256 = 0x10u;
    DAT_80143bb0 = 0u;
    DAT_80143b90 = 2u;
  }

  func_80199230();
}
