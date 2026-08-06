#include "internal.h"

/* @behavior clears one battler bit in the shared pending-bitset at `0x801463c2`.
 * @source 0x801DE1B0
 */
extern u16 D_801463C2;

void battle03_clear_pending_bit(u32 arg0) {
  u16* pending_bits = &D_801463C2;

  *pending_bits &= (u16) ~(1u << arg0);
}
