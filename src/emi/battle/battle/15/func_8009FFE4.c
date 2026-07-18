#include "internal.h"

/* @stores 0x-5 to offset 2 of the pointer loaded from D_801463A0.
 * @source 0x8009FFE4
 */
void func_8009FFE4(void) {
  volatile u16* ptr;

  ptr = (volatile u16*)D_801463A0;
  *(ptr + 2) = -5u;
}
