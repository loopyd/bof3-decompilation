#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @increments byte at offset 1 of the scratchpad pointer if D_801462E8 has bit 2 set.
 * Uses volatile casts for scratchpad access (0x1F80xxxx).
 * @source 0x800AE438
 */
void func_800AE438(void) {
  void* sp_ptr;

  if (D_801462E8 & 4) {
    sp_ptr = *(void**)SPAD_ADDRESS(0x44u);
    (*(volatile u8*)((u32)sp_ptr + 1))++;
  }
}
