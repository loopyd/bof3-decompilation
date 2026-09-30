#include "bof3/battle/battle03_internal.h"

/* @source 0x801E4544
 * @behavior Counts down scratch work byte +0x0A, reloading it to 4 and advancing work byte +0x03 when it expires, then advances the work position pair +0x34/+0x38 by the delta pair +0x0C/+0x10 and runs enemyReadyOrHelper1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownWorkByteAdvancePairEnemyReady(void)
{
    if (--D_1F800044->pad_09[1] == 0u) {
        D_1F800044->pad_09[1] = 4;
        D_1F800044->unk_03++;
    }
    D_1F800044->unk_34 += D_1F800044->unk_0c;
    D_1F800044->unk_38 += D_1F800044->unk_10;
    enemyReadyOrHelper1();
}
