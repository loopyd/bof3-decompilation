#include "internal.h"

/* @behavior clears one battler bit in the shared pending-bitset at `0x801463c2`.
 * @source 0x801de1b0 FUN_801de1b0
 */
extern u16 DAT_801463c2;

void func_801de1b0(u32 arg0) {
  u16* pending_bits = &DAT_801463c2;

  *pending_bits &= (u16) ~(1u << arg0);
}
