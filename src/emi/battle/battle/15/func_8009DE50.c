#include "internal.h"

/* @stores 0x-14 to offset 2 of the pointer loaded from D_801463A0.
 * @source 0x8009DE50
 */
void func_8009DE50(void) {
  volatile u16* ptr;

  ptr = (volatile u16*)D_801463A0;
  *(ptr + 2) = -20u;
}
