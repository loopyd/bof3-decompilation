#include "bof3/bof3.h"

extern u8 D_80144F5A; /* @source 0x80144F5A @kind data */
extern u8 D_80146865; /* @source 0x80146865 @kind data */

/* @source 0x801F2F94
 * @behavior Stores zero into the shared byte at 0x80146865 when the byte at
 *           0x80144F5A equals four; otherwise stores one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSharedByteZeroIfByte80144F5AFourElseOneWorld01Area03913_801F2F94(void) {
  if (D_80144F5A == 4) {
    D_80146865 = 0;
  } else {
    D_80146865 = 1;
  }
}
