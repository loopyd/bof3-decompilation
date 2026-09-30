#include "bof3/bof3.h"

extern u8 D_801462ED;
extern u8 D_80146375;
extern u8 D_80146384;

/* @source 0x800C2128
 * @behavior Handler slot 0 of the 0x800C3294 table that the sibling
 * dispatcher at 0x800C20F0 selects from. While bit 3 (0x08) of the boss flag
 * byte 0x801462ED is clear it stores the constant 3 into the global byte at
 * 0x80146384; when that bit is set it stores the constant 0 into the global
 * byte at 0x80146375 instead. It reads no other state and returns no value.
 * Not reached by a trailing jump: the dispatcher's table holds its address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeThreeOrZeroByFlagBitThreeBossBoss02513_800C2128(void) {
  if ((D_801462ED & 0x08) == 0) {
    D_80146384 = 3;
  } else {
    D_80146375 = 0;
  }
}
