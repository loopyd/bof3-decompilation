#include "internal.h"

/* @source 0x801E30B8
 * @behavior Returns 0xFF when D_801462F3 is 1; otherwise calls battle03_pick_random_unblocked_id with arg0 + 3 as u8.
 */
u8 battle03_pick_target_or_ff(s8 arg0) {
    s32 result;

    if (D_801462F3 == 1) {
        result = 0xFF;
    } else {
        result = battle03_pick_random_unblocked_id((u8)(arg0 + 3));
    }

    return result;
}
