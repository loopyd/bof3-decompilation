#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores 0x-F0 to offset 2 of the pointer loaded from D_801463A0.
 * @source 0x8009FE98
 */
void func_8009FE98(void) {
  volatile u16* ptr;

  ptr = (volatile u16*)D_801463A0;
  *(ptr + 2) = -240u;
}
