#include "bof3/battle/battle03_internal.h"

extern u8 D_801463CD; /* @source 0x801463CD @kind unknown */

/* @source 0x801E4B2C
 * @behavior When the loader is ready, initializes enemy cue countdown byte +9,
 * advances work byte +1 or +2 according to selection mask 0x800, and checks
 * enemy readiness. The byte +2 path also selects script cue 6.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initEnemyCueCountdownWhenLoaderReady(void)
{
    Battle03LocalWork *work;

    if (func_80162D00() != 0) {
        if (battleCurrentEnemyWorkState->unk_f5 == 4) {
            if (BATTLE_GLOBAL_BYTE_62EA != 0) {
                if (scanTriggerTableSubmit(
                        (u8)(battleCurrentEnemyWorkState->unk_f0 + 0x20),
                        D_801463CD) == 0) {
                    D_1F800044->unk_09 = 30;
                } else {
                    D_1F800044->unk_09 = 0;
                }
            }
        } else {
            D_1F800044->unk_09 = 0;
        }
        if ((D_801CA71C[BATTLE_GLOBAL_HALF_63C0].mask_00 & 0x800) == 0) {
            D_1F800044->unk_09 = PSX_REF(
                u8, 0x800E40CFu + battleCurrentEnemyWorkState->unk_e0 * 0x88u);
            func_801E2314(6);
            work = D_1F800044;
            work->unk_02++;
        } else {
            work = D_1F800044;
            work->unk_01++;
        }
        enemyReadyOrHelper2();
    }
}
