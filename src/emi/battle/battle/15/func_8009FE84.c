#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores 0x-50 to offset 2 of the pointer loaded from D_801463A0.
 * @source 0x8009FE84
 */
void func_8009FE84(void) {
  s16* ptr;

  ptr = D_801463A0;
  *(ptr + 2) = -80;
}
