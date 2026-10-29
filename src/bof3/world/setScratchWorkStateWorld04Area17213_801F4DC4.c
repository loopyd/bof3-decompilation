#include "bof3/bof3.h"

/* @source 0x801F4DC4
 * @behavior Resets the overlay's scratchpad work object reached through the
 * pointer slot at 0x1F800044: clears the halfword at offset 0x2E and the
 * 32-bit counter at offset 0x0C, then writes 1 into the byte at offset 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScratchWorkStateWorld04Area17213_801F4DC4(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u16*)(work + 0x2e) = 0;
  *(u32*)(work + 0x0c) = 0;
  work[1] = 1;
}
