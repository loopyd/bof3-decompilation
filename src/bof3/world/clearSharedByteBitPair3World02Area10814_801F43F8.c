#include "bof3/bof3.h"

/* @source 0x80146865 @kind data */
extern u8 D_80146865;

/* @source 0x801F43F8
 * @behavior Clears bits 7 and 6 of the shared byte at 0x80146865,
 * preserving bits 5 through 0 with mask 0x3F.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedByteBitPair3World02Area10814_801F43F8(void) {
  u8* shared_byte = &D_80146865;

  *shared_byte &= 0x3Fu;
}
