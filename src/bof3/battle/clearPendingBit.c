#include "bof3/battle/battle03_internal.h"

/* @behavior clears one battler bit in the shared pending-bitset at `0x801463c2`.
 * @source 0x801DE1B0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
extern u16 D_801463C2;

void clearPendingBit(u32 arg0) {
  u16* pending_bits = &D_801463C2;

  *pending_bits &= (u16) ~(1u << arg0);
}
