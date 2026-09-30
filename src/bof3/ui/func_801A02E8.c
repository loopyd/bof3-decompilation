#include "bof3/ui/game00_internal.h"

/* @behavior searches the eleven u16 context-seed entries of D_801C8384 for the
 * current context seed D_80143F10, stores request kind 11 (0xB) into the pending
 * request kind byte D_80143F1D on a hit, and otherwise stores 1 after the table
 * has been exhausted.
 * @source 0x801A02E8
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit is instruction- and byte-exact: 24/24 instructions, 96 bytes.
 * The seed is read through a non-volatile view (`*(u16*)&D_80143F10`) because
 * the original materialises exactly one non-volatile `lhu` in the loop
 * preheader, before the hoisted `li 11` store constant and the table base, and
 * compares that raw halfword; the reviewed `volatile` declaration would force a
 * per-iteration reload plus an `andi` halfword promotion.
 */
void func_801A02E8(void) {
  s32 i;
  u16 seed;

  i = 0;
  seed = *(u16*)&D_80143F10;
  for (; i < 11; i++) {
    if (seed == D_801C8384[i]) {
      D_80143F1D = 11;
      return;
    }
  }
  if (i == 11) {
    D_80143F1D = 1;
  }
}
