#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores 0x-64 to offset 2 of the pointer loaded from D_801463A0.
 * @source 0x8009DE78
 */
void func_8009DE78(void) {
  s16* ptr;

  ptr = (s16*)D_801463A0;
  *(ptr + 2) = -100;
}
