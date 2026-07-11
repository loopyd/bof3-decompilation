#include "internal.h"

/* @behavior oscillates the UI byte at `0x80146308` between rising and falling modes
 * using the latch byte at `0x80146304`.
 * @source 0x801dd800 FUN_801dd800
 */
void func_801dd800(void) {
  volatile u8* value_ptr;
  u8           value;
  u8           next;

  value_ptr = (volatile u8*)(0x80140000u + 0x6308u);
  value = *value_ptr;
  if (value > 0x1eu) {
    *(value_ptr - 4) = 1u;
  }
  if (value == 0u) {
    *(value_ptr - 4) = 0u;
  }
  next = value + 2u;
  if (*(value_ptr - 4) == 1u) {
    next = value - 2u;
  }
  *value_ptr = next;
}
