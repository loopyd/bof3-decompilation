#include "internal.h"

/* does: seeds one spinning local work entry from scratchpad position, applies
 * three random Euler rotations to two fixed edge vectors, and clears the later
 * velocity/fade slots.
 * @source: 0x801f3be4 FUN_801f3be4
 */
void func_801f3be4(void* arg0) {
  World00Area024SpinWork* work;
  MATRIX                  matrix;
  s16                     angle_x;
  s16                     angle_y;
  s16                     angle_z;

  work = (World00Area024SpinWork*)arg0;

  work->field_00 = BOF3_WORLD00_AREA024_SCRATCH_PTR->field_34;
  work->field_04 = BOF3_WORLD00_AREA024_SCRATCH_PTR->field_38;
  work->field_08 = BOF3_WORLD00_AREA024_SCRATCH_PTR->field_3c;

  angle_x = rand() & 0xfff;
  angle_y = rand() & 0x3ff;
  angle_z = rand() & 0xfff;

  work->field_10.vx = rcos(0x10);
  work->field_10.vy = rsin(0x10);
  work->field_10.vz = 0;
  work->field_18.vx = rcos(-0x10);
  work->field_18.vy = rsin(-0x10);
  work->field_18.vz = 0;

  func_801aff64(&matrix);
  RotMatrixX(angle_x, &matrix);
  RotMatrixY(-angle_y, &matrix);
  RotMatrixZ(angle_z, &matrix);

  PushMatrix();
  ApplyMatrixSV(&matrix, &work->field_10, &work->field_10);
  ApplyMatrixSV(&matrix, &work->field_18, &work->field_18);
  PopMatrix();

  work->field_28 = (rand() & 3) + 0xa;
  work->field_20 = 0;
  work->field_22 = 0;
  work->field_24 = 0;
  work->field_2a = 0;
}
