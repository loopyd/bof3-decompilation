#include "bof3/battle/battle03_internal.h"

/* @source 0x801E483C
 * @behavior Runs the front-end cue 0x204, then, while the battle gate byte
 * 0x801462EA is set, resubmits the current enemy work effect halfword +0x6 of
 * the record at +0xE8 through submitEffectUnlessNeg1. Submits the scratch work
 * record through func_80196718 and its positional effect through
 * func_8019651C with the 0xFF/0xFF/0xFF offsets and mode 0, seeds the scratch
 * work acceleration +0x1C with -0x2000, clears its velocity +0x10 and advances
 * scratch work byte +0x03 to 3, the state that applyGravityOrReset integrates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitWorkAndSeedGravity(void)
{
    func_8015DF18(0x204);
    if (BATTLE_GLOBAL_BYTE_62EA != 0) {
        submitEffectUnlessNeg1(battleCurrentEnemyWorkState->unk_e8[3]);
    }
    func_80196718(D_1F800044);
    func_8019651C(D_1F800044, 0xFF, 0xFF, 0xFF, 0);
    D_1F800044->unk_1c = -0x2000;
    D_1F800044->unk_10 = 0;
    D_1F800044->unk_03 = 3;
}
