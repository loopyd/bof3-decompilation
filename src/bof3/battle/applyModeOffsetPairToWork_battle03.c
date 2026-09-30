#include "bof3/battle/battle03_internal.h"

/* @source 0x801E347C
 * @behavior Clears scratch work flag bit 6, runs the enemy script byte helper func_801E2314 for class 0, mirrors the work pair +0x34/+0x38 into +0x18/+0x1C, subtracts the mode-rotated 8.0 offset pair from +0x34, republishes the mode-rotated 0.5 offset pair at +0x0C/+0x10, then advances scratch work byte +0x02.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyModeOffsetPairToWork(void)
{
    Battle03LocalWork *first_work;
    Battle03LocalWork *pair_work;
    Battle03LocalWork *offset_work;
    Battle03LocalWork *counter_work;
    s32                mirror_x;
    s32                mirror_y;
    s32                pair_x;
    s32                pair_y;
    s32                pair_offset;

    first_work = D_1F800044;
    first_work->flags_00 &= 0xBF;
    func_801E2314(0);

    pair_work = D_1F800044;
    mirror_x = pair_work->unk_34;
    mirror_y = pair_work->unk_38;
    pair_work->unk_0c = 0x80000;
    pair_work->unk_10 = 0;
    pair_work->unk_18 = mirror_x;
    pair_work->unk_1c = mirror_y;
    transformFirstPointPairByMode((s32)pair_work);

    offset_work = D_1F800044;
    pair_offset = offset_work->unk_34;
    pair_x = offset_work->unk_0c;
    pair_y = offset_work->unk_10;
    offset_work->unk_0c = 0x8000;
    offset_work->unk_10 = 0;
    pair_offset -= pair_x;
    /* The shared scratch pair keeps both original stores: the intermediate value is
     * published through a volatile view so the earlier store is not elided. */
    ((volatile Battle03LocalWork *)offset_work)->unk_34 = pair_offset;
    pair_offset -= pair_y;
    offset_work->unk_34 = pair_offset;
    transformFirstPointPairByMode((s32)offset_work);

    counter_work = D_1F800044;
    counter_work->unk_02++;
}
