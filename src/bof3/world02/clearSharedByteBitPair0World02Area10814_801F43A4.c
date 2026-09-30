#include "bof3/bof3.h"

extern u8 D_80146865;

/* @source 0x801F43A4
 * @behavior Overlay handler: clears the low bit pair (bits 1 and 0, mask
 * 0xFC) of the shared state byte at 0x80146865 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedByteBitPair0World02Area10814_801F43A4(void) {
  u8* shared_byte = &D_80146865;

  *shared_byte &= 0xFCu;
}
