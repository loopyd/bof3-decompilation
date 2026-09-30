#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E0B6C
 * @behavior Sums the AREA030 summary value of every slot that is currently
 * carrying an amount: it walks the 32 shared amount bytes at 0x80144FE4 and
 * adds the value reported by func_801E0ABC (0x801E0ABC) for that slot index
 * whenever the amount byte is nonzero, then reports the sum masked to 16 bits.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 25/25 instructions, 100 bytes, live byte match with no matching
 * aids. The first live asm-diff of the first-seed source was already exact:
 * the early-skip `if (amount != 0)` body keeps the byte load in $a1 (the
 * callee's second argument), the `i & 0xff` mask supplies the callee's slot
 * index and the `sum & 0xffff` return mask lands in the loop's delay slot.
 */
u32 func_801E0B6C(void) {
  s32 i;
  s32 sum = 0;

  for (i = 0; i < 0x20; i++) {
    u8 amount = D_80144FE4[i];

    if (amount != 0) {
      sum += func_801E0ABC(i & 0xff, amount);
    }
  }
  return sum & 0xffff;
}
