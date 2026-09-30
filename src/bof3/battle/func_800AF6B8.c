#include "bof3/battle/battle15_internal.h"

/* @behavior Tests a size word against both scratchpad work ranges' 0x34/0x38
 * origins plus the 0x3E height word, returning 0 unless the size fits both
 * ranges, then reporting whether the size-based distance still fits inside the
 * 0x3E word's low half.
 * @source 0x800AF6B8
 * @status partial
 * @match 25.93
 * @residual first mismatch +0x0004: the original hoists `move v0,zero` into the
 * entry block and shares one return, while the explicit early `return result;`
 * here yields a separate zero-return block plus an extra 4-byte `j` (27 insns
 * vs 26; 104->108 bytes); the plane pseudo lands in a2 where the original used
 * t2. Clean-C retry: assign inside the if and use a single trailing
 * `return result;`. Ladder tried: cfg/return shapes, statement orders, bounded
 * permute, profile probes; no aids. Byte match and independent review pending.
 */
u32 func_800AF6B8(u32 x, u32 y, s32 z, u32 size) {
    BattleWork *work;
    u32 result;
    u32 plane;
    u32 half_size;
    u32 first;
    u32 second;

    result = 0;
    work = (BattleWork *)g_battle_work;
    half_size = size >> 1;
    plane = work->unk_3E + (size >> 9) - (z >> 16);
    first = work->range_axis_34 + half_size - x;
    second = work->range_axis_38 + half_size - y;
    if (size >= first && size >= second) {
        return (plane & 0xFFFF) <= (size >> 8);
    }
    return result;
}
