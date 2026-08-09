#include "bof3/battle/battle03_internal.h"

/* @source 0x801E3334
 * @behavior Clears local-work flag bit 6, invokes func_801E2314(0), and initializes scratchpad work state.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void initScratchWorkState(void)
{
    Battle03LocalWork *first_work;
    Battle03LocalWork *second_work;

    first_work = D_1F800044;
    first_work->flags_00 &= 0xBF;
    func_801E2314(0);
    D_1F800044->unk_48 = 2;
    second_work = D_1F800044;
    second_work->unk_18 = 0x666;
    second_work->unk_44 = 0;
    second_work->unk_40 = 0;
    second_work->unk_0c = 0;
    second_work->unk_03 = 1;
}
