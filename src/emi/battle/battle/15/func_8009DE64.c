#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores 0x-28 to offset 2 of the pointer loaded from D_801463A0.
 * @source 0x8009DE64
 */
void func_8009DE64(void) {
  volatile u16* ptr;

  ptr = (volatile u16*)D_801463A0;
  *(ptr + 2) = -40u;
}
