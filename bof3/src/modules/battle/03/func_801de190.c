#include "internal.h"

/* does: marks one battler bit in the shared pending-bitset at `0x801463c2`.
 * @source: 0x801de190 FUN_801de190
 */
void func_801de190(u32 arg0) { *(u16*)0x801463c2 |= 1 << arg0; }
