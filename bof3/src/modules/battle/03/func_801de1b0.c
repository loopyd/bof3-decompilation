#include "internal.h"

/* does: clears one battler bit in the shared pending-bitset at `0x801463c2`.
 * @source: 0x801de1b0 FUN_801de1b0
 */
void func_801de1b0(u32 arg0) { (*(u16*)0x801463c2u) &= (u16) ~(1u << arg0); }
