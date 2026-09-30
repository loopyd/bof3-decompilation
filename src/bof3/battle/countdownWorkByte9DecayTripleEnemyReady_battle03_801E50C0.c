#include "bof3/battle/battle03_internal.h"

/* @source 0x801E50C0
 * @behavior Counts down scratch work byte +0x09 and returns while it is nonzero; otherwise counts down work byte +0x0A, advancing work byte +0x02 when that expires and otherwise stepping the work triple +0x5D/+0x5E/+0x5F down by 0x10, integrating the motion pair +0x0C/+0x10 into +0x34/+0x38 and running enemyReadyOrHelper1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownWorkByte9DecayTripleEnemyReady(void)
{
    if (D_1F800044->pad_09[0] != 0) {
        D_1F800044->pad_09[0]--;
    } else if (--D_1F800044->pad_09[1] != 0u) {
        D_1F800044->unk_5d -= 0x10;
        D_1F800044->unk_5e -= 0x10;
        D_1F800044->unk_5f -= 0x10;
        D_1F800044->unk_0c += D_1F800044->unk_18;
        D_1F800044->unk_10 += D_1F800044->unk_1c;
        D_1F800044->unk_34 += D_1F800044->unk_0c;
        D_1F800044->unk_38 += D_1F800044->unk_10;
        enemyReadyOrHelper1();
    } else {
        D_1F800044->unk_02++;
    }
}
