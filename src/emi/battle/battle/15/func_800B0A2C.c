#include "internal.h"

/* @stores halfword 0x5A at offset 4, halfword -0x17 at offset 6 of D_80148648,
 * and increments byte at offset 3.
 * @source 0x800B0A2C
 */
void func_800B0A2C(void) {
  volatile u8* ptr;

  ptr = (volatile u8*)D_80148648;
  (*(volatile u16*)((u32)ptr + 4)) = 0x5A;
  (*(volatile u16*)((u32)ptr + 6)) = -0x17;
  ptr[3]++;
}
