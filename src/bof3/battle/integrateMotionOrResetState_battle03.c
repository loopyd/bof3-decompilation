#include "bof3/battle/battle03_internal.h"

/* @source 0x801E3398
 * @behavior Integrates the scratch work motion pair while signed +0x40 stays at
 * or below 0xFFFF; otherwise clears work byte +0x48 and resets the scratch
 * dispatch state to 3/0/0, then runs enemyReadyOrHelper1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void integrateMotionOrResetState(void) {
  Battle03LocalWork* work;

  work = (Battle03LocalWork*)D_1F800044;
  if ((s32)work->unk_40 <= 0xFFFF) {
    work->unk_40 += work->unk_0c;
    work->unk_44 += work->unk_0c;
    work->unk_0c += work->unk_18;
  } else {
    work->unk_48 = 0;
    D_1F800044->unk_01 = 3;
    D_1F800044->unk_02 = 0;
    D_1F800044->unk_03 = 0;
  }
  enemyReadyOrHelper1();
}
