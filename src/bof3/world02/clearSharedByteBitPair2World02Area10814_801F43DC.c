#include "bof3/bof3.h"

extern u8 D_80146865;

/* @source 0x801F43DC
 * @behavior Overlay handler: clears the middle-high bit pair (bits 5 and 4,
 * mask 0xCF) of the shared state byte at 0x80146865 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedByteBitPair2World02Area10814_801F43DC(void) {
  u8* shared_byte = &D_80146865;

  *shared_byte &= 0xCFu;
}
