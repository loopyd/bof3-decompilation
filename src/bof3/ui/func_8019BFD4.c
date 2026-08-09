#include "bof3/ui/game00_internal.h"

/**
 * @source 0x8019BFD4
 * @behavior Copies two shared values into working state, initializes control
 * fields, and negates the signed value at D_80149330.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8019BFD4(void) {
  u32 value_0;
  u32 value_1;
  s32 signed_value;

  value_0 = D_80145EC4;
  value_1 = D_80145EC8;
  signed_value = D_80149330;
  D_8014932E = 0x40;
  D_80149332 = 1;
  D_80149308 = value_0;
  D_8014930C = value_1;
  D_80149330 = -signed_value;
}
