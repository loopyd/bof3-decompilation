#include "bof3/bof3.h"

extern u8 D_80144F5B;
extern u8 D_80146865;

/* @source 0x801F33A0
 * @behavior When the byte at 0x80144F5B reads 4, stores the constant 1 into
 *           the shared byte D_80146865; the shared byte is left alone
 *           otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSharedByteOneIfByte80144F5BFourWorld01Area04914_801F33A0(void) {
  if (D_80144F5B == 4) {
    D_80146865 = 1;
  }
}
