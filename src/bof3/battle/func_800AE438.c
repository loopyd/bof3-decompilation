#include "bof3/battle/battle15_internal.h"

/* @source 0x800AE438
 * @behavior If bit 0x0004 of the halfword at 0x801462E8 is set, increments byte
 * +0x01 through the pointer at 0x1F800044 modulo 256.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800AE438(void) {
  volatile u8* sp_byte;

  if (D_801462E8 & 4) {
    sp_byte = *(volatile u8**)SPAD_ADDRESS(0x44u);
    sp_byte[1] = sp_byte[1] + 1;
  }
}
