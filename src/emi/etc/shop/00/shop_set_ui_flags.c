#include "internal.h"

/* @source 0x801D66AC
 * @behavior sets D_80148650 to 1 and D_80148651 to 0.
 */
void shop_set_ui_flags(void) {
  D_80148650 = 1;
  D_80148651 = 0;
}
