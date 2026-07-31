#include "internal.h"

/* @behavior Checks horizontal and vertical battle-work bounds for arg1.
 * @source 0x800AF66C
 */
u32 func_800AF66C(u8 *arg0, u32 arg1) {
    u8 *battle_work;
    u32 half_arg1;

    battle_work = g_battle_work;
    half_arg1 = arg1 >> 1;
    if (arg1 < FIELD_REF(u32, battle_work, 0x34) + half_arg1 - FIELD_REF(u32, arg0, 0x34)) {
        return 0;
    }
    return arg1 >= FIELD_REF(u32, battle_work, 0x38) + half_arg1 - FIELD_REF(u32, arg0, 0x38);
}
