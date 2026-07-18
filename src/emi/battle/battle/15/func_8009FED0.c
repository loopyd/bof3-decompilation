#include "internal.h"

/* @stores byte 2 at offset 8 and halfword -0x28 at offset 6 of the pointer from D_801463A0.
 * @source 0x8009FED0
 */
void func_8009FED0(void) {
  volatile u8* ptr;

  ptr = (volatile u8*)D_801463A0;
  *(ptr + 8) = 2;
  (*(volatile u16*)((u32)ptr + 6)) = -0x28;
}
