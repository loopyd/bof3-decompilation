#include "bof3/battle/battle03_internal.h"

/* @source 0x801E3A44
 * @behavior Calls the enemy readiness helper, decrements scratch work byte +9,
 * and on reaching zero submits the enemy's first effect or its doubled cue
 * byte combined with 0x600, then advances scratch work byte +2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9SubmitEnemyCue(void)
{
    Battle03LocalWork *work;

    enemyReadyOrHelper1();
    work = D_1F800044;
    if (--work->unk_09 == 0) {
        if (BATTLE_GLOBAL_BYTE_62EA != 0) {
            submitEffectUnlessNeg1(battleCurrentEnemyWorkState->unk_e8[0]);
        } else {
            func_8015DF18((battleCurrentEnemyWorkState->unk_e0 << 1) | 0x600);
        }
        work = D_1F800044;
        work->unk_02++;
    }
}
