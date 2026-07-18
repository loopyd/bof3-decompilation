#include "internal.h"

/* @stores 0x-64 to offset 2 of the pointer loaded from D_801463A0.
 * @source 0x8009DE78
 */
void func_8009DE78(void) {
  volatile u16* ptr;

  ptr = (volatile u16*)D_801463A0;
  *(ptr + 2) = -100u;
}
