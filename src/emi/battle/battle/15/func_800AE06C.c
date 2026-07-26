#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @sets a byte at offset 9 of the scratchpad pointer, then increments 
 * byte at offset 1 of that same pointer.
 * Uses volatile casts for scratchpad access (0x1F80xxxx).
 * @source 0x800AE06C
 */
void func_800AE06C(void) {
  void* sp_ptr;

  SPAD_REF(u8, 0x44u) = 0x3C;
  sp_ptr = *(void**)SPAD_ADDRESS(0x44u);
  (*(volatile u8*)((u32)sp_ptr + 1))++;
}
