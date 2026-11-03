#include "bof3/battle/battle03_internal.h"

/* @source 0x801E352C
 * @behavior Integrates the scratch work delta pair +0x0C/+0x10 into the
 * position pair +0x34/+0x38 and publishes the signed distance of that pair
 * through func_8015477C in work halfword +0x3E; once the position pair equals
 * the mirrored target pair +0x18/+0x1C it reseeds the scratch state bytes
 * +0x01..+0x03 with 3/0/0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void integratePositionPairUntilTargetThenResetState(void) {
  Battle03LocalWork* motion_work;
  Battle03LocalWork* distance_work;
  s16                distance;

  enemyReadyOrHelper1();
  motion_work = D_1F800044;
  motion_work->unk_34 += motion_work->unk_0c;
  motion_work->unk_38 += motion_work->unk_10;
  distance = func_8015477C(motion_work->unk_34, motion_work->unk_38);
  distance_work = D_1F800044;
  distance_work->unk_3e = distance;
  if (distance_work->unk_34 == distance_work->unk_18 &&
      distance_work->unk_38 == distance_work->unk_1c) {
    D_1F800044->unk_01 = 3;
    D_1F800044->unk_02 = 0;
    D_1F800044->unk_03 = 0;
  }
}
