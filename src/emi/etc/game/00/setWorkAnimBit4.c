#include "internal.h"

/* @behavior sets bit 4 of arg0+0x74 unless the scratch work byte at 0x06 is 8.
 * @source 0x801A1A24
 */
void setWorkAnimBit4(u8* arg0) {
  struct GameWorkArea* work = SPAD_PTR_SLOT(struct GameWorkArea, 0x44u);

  if (work->unk_06 != 8) {
    arg0[0x74] |= 0x10;
  }
}
