#include "internal.h"

/* @behavior marks one battler bit in the shared pending-bitset at `0x801463c2`.
 * @source 0x801de190 FUN_801de190
 */
extern u16 D_801463C2;

void func_801de190(u32 arg0) {
  u16* pending_bits = &D_801463C2;

  *pending_bits |= 1 << arg0;
}
