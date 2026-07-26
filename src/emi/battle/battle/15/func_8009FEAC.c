#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores byte 2 at offset 8 and halfword -5 at offset 6 of the pointer from D_801463A0.
 * @source 0x8009FEAC
 */
void func_8009FEAC(void) {
  volatile u8* ptr;

  ptr = (volatile u8*)D_801463A0;
  *(ptr + 8) = 2;
  (*(volatile u16*)((u32)ptr + 6)) = -5;
}
