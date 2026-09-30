#include "bof3/battle/battle03_internal.h"

/* @source 0x801E44D4
 * @behavior Clears the current enemy work halfword +0xF8, publishes the scratch
 * work pair +0x0C/+0x10 as -0x2000/0, mode-transforms that pair, reloads scratch
 * work byte +0x0A to 4, runs enemyReadyOrHelper1 and func_8015DF18(0x205), then
 * advances scratch work byte +0x03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E44D4(void) {
  Battle03LocalWork* work;

  battleCurrentEnemyWorkState->unk_f8 = 0;
  work = D_1F800044;
  work->unk_0c = -0x2000;
  work->unk_10 = 0;
  transformFirstPointPairByMode((s32)work);
  D_1F800044->pad_09[1] = 4;
  enemyReadyOrHelper1();
  func_8015DF18(0x205);
  D_1F800044->unk_03++;
}
