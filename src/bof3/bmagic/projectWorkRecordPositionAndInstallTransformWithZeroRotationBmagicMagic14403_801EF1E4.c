#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF1E4
 * @behavior Pushes the matrix stack and reads the scratchpad work-record pointer
 *           once. Builds a position triplet from record words 0x34 and 0x38
 *           shifted right by nine and biased by minus 0x4000, and half the
 *           negated signed halfword 0x3E. RotTrans projects that position into
 *           the scratch matrix translation; RotMatrix builds its rotation from
 *           a zero triplet. Composes the shared matrix at 0x801492E8 with it
 *           using MulMatrix2, then installs its rotation and translation.
 *           Takes no arguments and returns nothing; the caller at 0x801EEEE0
 *           pairs this setup with PopMatrix after invoking 0x801EF294.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformWithZeroRotationBmagicMagic14403_801EF1E4(
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
