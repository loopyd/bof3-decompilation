#include "bof3/world/area02414_internal.h"

/* @source 0x801F3BE4
 * @behavior per-entry initializer of the local 16-entry spin-work table called
 * by initSpinWork: copies the scratchpad work object's 0x34/0x38/0x3c words
 * into the record's leading words, seeds its two SVECTOR components from the
 * shared angular helpers func_801783C8/func_801782FC at +0x10 and -0x10, builds
 * a scratch rotation matrix from three masked rand() angles (0xfff/0x3ff/0xfff,
 * the second negated) handed to func_801AFF64, func_80179C58, func_80179DF8 and
 * func_80179F98, rotates both SVECTOR components of the record through that
 * matrix between a PushMatrix/PopMatrix pair, then arms the record's 0x28
 * factor to rand()&3 + 0xa and clears its 0x20/0x22/0x24/0x2a halfwords.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3BE4(void* arg0) {
  World00Area024SpinWork* work;
  World00Area024Scratch*  scratch;
  MATRIX                  matrix;
  s16                     angle[3];

  work = (World00Area024SpinWork*)arg0;
  scratch = (World00Area024Scratch*)D_1F800044;

  work->field_00 = scratch->field_34;
  work->field_04 = scratch->field_38;
  work->field_08 = scratch->field_3c;

  angle[0] = (s16)(rand() & 0xfff);
  angle[1] = (s16)(rand() & 0x3ff);
  angle[2] = (s16)(rand() & 0xfff);

  work->field_10.vx = (s16)func_801783C8(0x10);
  work->field_10.vy = (s16)func_801782FC(0x10);
  work->field_10.vz = 0;
  work->field_18.vx = (s16)func_801783C8(-0x10);
  work->field_18.vy = (s16)func_801782FC(-0x10);
  work->field_18.vz = 0;

  func_801AFF64(&matrix);
  func_80179C58(angle[0], &matrix);
  func_80179DF8(-angle[1], &matrix);
  func_80179F98(angle[2], &matrix);

  PushMatrix();
  ApplyMatrixSV(&matrix, &work->field_10, &work->field_10);
  ApplyMatrixSV(&matrix, &work->field_18, &work->field_18);
  PopMatrix();

  work->field_28 = (s16)((rand() & 3) + 0xa);
  work->field_20 = 0;
  work->field_22 = 0;
  work->field_24 = 0;
  work->field_2a = 0;
}
