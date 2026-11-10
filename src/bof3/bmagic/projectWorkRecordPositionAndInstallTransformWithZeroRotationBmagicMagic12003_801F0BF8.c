#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801F0BF8
 * @behavior Push the matrix stack and read the work pointer at scratchpad
 *           0x1F800044 once. Form a position triplet from work words 0x34 and
 *           0x38 shifted right nine and biased by -0x4000, and half the
 *           negated signed halfword 0x3E. Project it with RotTrans into a
 *           local matrix translation, build a zero-angle rotation in that
 *           matrix, compose the shared matrix at 0x801492E8 with MulMatrix2,
 *           and install the result with SetRotMatrix and SetTransMatrix.
 *           Takes no arguments and returns nothing; record fields remain
 *           offset-anchored. A payload cmp proved this 0xB0-byte span equal
 *           to magic118/03 at 0x801EF60C, which supplied the C shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformWithZeroRotationBmagicMagic12003_801F0BF8(
    void) {
  u8 *work;
  s16 rotation[3];
  s16 translation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();

  work = g_battle_work;
  translation[0] = 0;
  translation[1] = 0;
  translation[2] = 0;
  rotation[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  rotation[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  rotation[2] = (s16)(-*(s16 *)(work + 0x3E) / 2);
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
