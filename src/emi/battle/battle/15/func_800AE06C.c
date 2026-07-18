#include "internal.h"

/* @sets a byte at offset 9 of the scratchpad pointer, then increments 
 * byte at offset 1 of that same pointer.
 * Uses volatile casts for scratchpad access (0x1F80xxxx).
 * @source 0x800AE06C
 */
void func_800AE06C(void) {
  void* sp_ptr;

  (*(volatile u8*)0x1F800044u) = 0x3C;
  sp_ptr = *(void**)0x1F800044u;
  (*(volatile u8*)((u32)sp_ptr + 1))++;
}
