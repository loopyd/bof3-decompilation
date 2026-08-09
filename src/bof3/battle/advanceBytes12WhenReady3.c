#include "bof3/battle/battle03_internal.h"

/* @source 0x801E4D34
 * @behavior Resets scratchpad work byte +2 after enemyReadyOrHelper2 succeeds, incrementing byte +1.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advanceBytes12WhenReady3(void)
{
    Battle03LocalWork *work;

    if (enemyReadyOrHelper2() != 0) {
        work = D_1F800044;
        work->unk_01++;
        D_1F800044->unk_02 = 0;
    }
}
