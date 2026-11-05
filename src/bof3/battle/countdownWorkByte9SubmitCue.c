#include "bof3/battle/battle03_internal.h"

/* @source 0x801E4C7C
 * @behavior Counts down scratch work byte +9; at zero submits an enemy-selected cue and advances byte +2, then checks enemy readiness.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownWorkByte9SubmitCue(void)
{
    Battle03LocalWork *work;
    u8 countdown;

    work = D_1F800044;
    countdown = work->unk_09;
    if (countdown == 0) {
        if (BATTLE_GLOBAL_BYTE_62EA != 0) {
            submitEffectUnlessNeg1(battleCurrentEnemyWorkState->unk_e8[1]);
        } else {
            func_8015DF18((battleCurrentEnemyWorkState->unk_e0 << 1) | 0x601);
        }
        work = D_1F800044;
        work->unk_02++;
    } else {
        work->unk_09 = countdown - 1;
    }
    enemyReadyOrHelper2();
}
