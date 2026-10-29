#include "bof3/bof3.h"

extern u8 D_80146865;

/* @source 0x801F43C0
 * @behavior Overlay handler: clears the middle-low bit pair (bits 3 and 2,
 * mask 0xF3) of the shared state byte at 0x80146865 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedByteBitPair1World02Area10814_801F43C0(void) {
  u8* shared_byte = &D_80146865;

  *shared_byte &= 0xF3u;
}
