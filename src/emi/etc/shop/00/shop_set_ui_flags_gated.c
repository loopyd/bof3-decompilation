#include "internal.h"

/* @source 0x801D66C8
 * @behavior conditionally sets D_80148650 to 1 and D_80148651 to 0 when bit 1 of
 *         D_801490A4 is set.
 */
void shop_set_ui_flags_gated(void) {
  if (D_801490A4 & 2) {
    D_80148650 = 1;
    D_80148651 = 0;
  }
}
