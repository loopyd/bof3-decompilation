#include "bof3/bof3.h"

extern u8 D_80144F5C;
extern u8 D_80146866;

/* @source 0x801F33C4
 * @behavior When the byte at 0x80144F5C reads 4, stores the constant 1 into
 *           the shared byte D_80146866; the shared byte is left alone
 *           otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSharedByteOneIfByte80144F5CFourWorld01Area04914_801F33C4(void) {
  if (D_80144F5C == 4) {
    D_80146866 = 1;
  }
}
