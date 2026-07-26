#include "bof3/bof3.h"

extern u32 D_80181B10;

/* @source 0x801DD7AC
 * @behavior returns one indexed state byte, mapping value seven to zero.
 */
u8 func_801DD7AC(s32 arg0) {
  u8 var = *(volatile u8*)(D_80181B10 + (arg0 & 0xFF));

  if ((var & 0xFF) == 7) {
    var = 0;
  }
  return var;
}
