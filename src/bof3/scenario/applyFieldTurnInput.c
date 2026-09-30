#include "bof3/scenario/scena00_internal.h"

/**
 * @source 0x801FBE88
 * @behavior Points the scratchpad delta cell at 0x1F800010 at the delta pair
 * of the current world record, clears the first axis delta, folds the held
 * face-button bits of D_80145AA8 into the two signed halfwords of that pair,
 * steps the turn accumulator of the matching axis (D_80147BEC / D_80147BF0)
 * by 0x40, and queues frontend cue 0x204.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the delta cell reloads on every
 * evaluation immediately after its store (volatile cell), the pad word is read
 * through one materialized base, and each axis accumulator is addressed
 * through its own local to keep the address in a register across the
 * load/add/store triple.
 */
void applyFieldTurnInput(void)
{
    u16 *pad = &D_80145AA8;

    D_1F800010 = (u16 *)(*(u8 **)((u8 *)D_8017F974[D_80143F00] + 0xC) + 0x12);
    D_1F800010[0] = 0;

    if (*pad & 0x2000) {
        s32 *turn_x = &D_80147BEC;

        D_1F800010[0] += 2;
        *turn_x += 0x40;
    }
    if (*pad & 0x8000) {
        s32 *turn_x = &D_80147BEC;

        D_1F800010[0] -= 2;
        *turn_x -= 0x40;
    }

    D_1F800010[1] = 0;

    if (*pad & 0x4000) {
        s32 *turn_y = &D_80147BF0;

        D_1F800010[1] += 2;
        *turn_y += 0x40;
    }
    if (*pad & 0x1000) {
        s32 *turn_y = &D_80147BF0;

        D_1F800010[1] -= 2;
        *turn_y -= 0x40;
    }

    game_queue_frontend_cue(0x204);
}
