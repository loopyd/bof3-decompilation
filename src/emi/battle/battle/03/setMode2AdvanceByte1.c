#include "internal.h"

/* @source 0x801E7528
 * @behavior writes mode two to the current global record and increments work byte one.
 */
void setMode2AdvanceByte1(void) {
  D_801EB4E0->unk_48 = 2;
  SPAD_PTR_SLOT(u8, 0x44u)[1]++;
}
