#include "internal.h"

/* @source 0x8009F9E0 */
/* @behavior Sets the state flag at +0x08 and resets the signed halfwords at +0x04/+0x06. */
void func_8009F9E0(void) {
  s16 *ptr;

  ((volatile u8 *)D_801463A0)[8] |= 2;
  ((volatile s16 *)D_801463A0)[2] = -5;
  ptr = D_801463A0;
  ptr[3] = -1;
}
