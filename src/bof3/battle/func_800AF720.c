#include "bof3/battle/battle15_internal.h"

/* @behavior Checks horizontal and vertical battle-work bounds for a coordinate pair.
 * @source 0x800AF720
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u32 func_800AF720(u32 x, u32 y, u32 size) {
    /*
     * MATCHING_AID: Unpinned clean C starts with move t0,zero instead of the
     * original move t0,a2; the result is allocated to t0 instead of v0 and
     * shifts the 68-byte allocator web. Declaration/order, temporary,
     * pointer-hoist, branch/return, canonical and installed historical-profile,
     * and bounded permuter rungs were non-exact. Pinning only result to v0 is
     * immediately live exact (17/17 instructions, 68 bytes). Remove when an
     * unpinned clean-C shape or selected compiler profile is byte-exact.
     */
    REGISTER_PIN(u32, result, "v0");
    u32 value;
    u32 half_value;
    BattleRange *work;
    u32 first;
    u32 second;

    value = size;
    result = 0;
    half_value = value >> 1;
    work = (BattleRange *)g_battle_work;
    first = work->range_axis_34 + half_value - x;
    second = work->range_axis_38 + half_value - y;
    if (value >= first) {
        result = value >= second;
    }
    return result;
}
