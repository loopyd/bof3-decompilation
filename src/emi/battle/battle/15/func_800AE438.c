#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @increments byte at offset 1 of the scratchpad pointer if D_801462E8 has bit 2 set.
 * Uses volatile casts for scratchpad access (0x1F80xxxx).
 * @source 0x800AE438
 */
void func_800AE438(void) {
  volatile u8* sp_byte;

  if (D_801462E8 & 4) {
    sp_byte = *(volatile u8**)SPAD_ADDRESS(0x44u);
    sp_byte[1] = sp_byte[1] + 1;
  }
}
