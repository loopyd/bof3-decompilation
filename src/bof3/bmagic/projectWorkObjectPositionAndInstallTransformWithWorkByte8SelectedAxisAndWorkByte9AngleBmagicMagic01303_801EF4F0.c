#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row. The original materializes it twice in this body - once into $a0 for
 * the work-byte-0x8 arm selection and the angle reads, and again into $v1 at the
 * join where the position triplet is built - and never keeps a second view
 * across the arm join, so the plain named cell read once per site carries both
 * loads. Declaration style follows the exact same-overlay siblings
 * projectWorkObjectPositionAndInstallTransformWithWorkByte9AngleBmagicMagic01303_801F00A8
 * and projectWorkObjectPositionAndInstallTransformWithWorkByteAAngleBmagicMagic09403_801EF654. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built work-object transform.
 * This target's generated undefined_syms_auto.txt owns the address through the
 * row D_801492E8, so no target-map row is needed. The only access is the matrix
 * base handed to MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF4F0
 * @behavior Pushes the GTE matrix stack and reads the scratchpad work-object
 *           pointer cell 0x1F800044 for the arm selection. The unsigned work
 *           byte 0x8 picks one of four rotation arms: 0 zeroes rotation element
 *           0 and sets element 1 to 0x1000 minus the angle, 1 zeroes element 1
 *           and sets element 0 to the angle, 2 zeroes element 0 and sets
 *           element 1 to the angle, and 3 zeroes element 1 and sets element 0 to
 *           0x1000 minus the angle, so byte 0x8 selects which of the two
 *           horizontal axes turns and whether the turn is the angle or its
 *           complement in the 0x1000-per-turn angle circle. The angle is
 *           work byte 0x9 shifted left by six, the countdown the exact sibling
 *           countDownWorkByte9ThenSeedAngleFromByteBAddHalvedRotatorStepsToWords34And38CopyWord3CSetWord14ToRandPlus128Word20ToMinus12ResetByte9To16AndAdvanceWorkByte2BmagicMagic01303_801EFE80
 *           decrements each step and re-arms to 0x10. Every arm falls through to
 *           a shared third rotation element set to zero, and the cell is read a
 *           second time at that join. It then builds the position triplet from
 *           the work object: words 0x34 and 0x38 shifted right by nine and
 *           biased by minus 0x4000, and half the negation of the signed
 *           halfword 0x3E plus 0x20, taken as a truncating signed division.
 *           RotTrans projects that triplet into the translation field of a
 *           0x20-byte scratch matrix while storing a flag word beside it,
 *           RotMatrix turns the selected rotation triplet into that matrix,
 *           MulMatrix2 composes the shared matrix at 0x801492E8 under it, and
 *           the result is published through SetRotMatrix and SetTransMatrix.
 *           Takes no arguments and returns nothing; the 0x50-byte frame keeps
 *           $ra at 0x4C and $s0 at 0x48 across the matrix calls. The payload
 *           names this body only through the stored `jal` word at 0x801EF064
 *           inside the textbin blob at 0x801EF054, a script/data table that
 *           carries no prototype, so the roles stay structural.
 * @status partial
 * @match 74.71
 * @residual provisional seed metadata; replaced with the measured tags before
 *           the lane gate.
 */
void projectWorkObjectPositionAndInstallTransformWithWorkByte8SelectedAxisAndWorkByte9AngleBmagicMagic01303_801EF4F0(
    void) {
  u8 *work;
  s16 objectPosition[3];
  s16 objectRotation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();

  work = g_battle_work;
  switch (work[0x8]) {
  case 0:
    objectRotation[0] = 0;
    objectRotation[1] = 0x1000 - (work[0x9] << 6);
    break;
  case 1:
    objectRotation[1] = 0;
    objectRotation[0] = work[0x9] << 6;
    break;
  case 2:
    objectRotation[0] = 0;
    objectRotation[1] = work[0x9] << 6;
    break;
  case 3:
    objectRotation[1] = 0;
    objectRotation[0] = 0x1000 - (work[0x9] << 6);
    break;
  }
  objectRotation[2] = 0;
  work = g_battle_work;
  objectPosition[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  objectPosition[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  objectPosition[2] = (s16)(-(*(s16 *)(work + 0x3E) + 0x20) / 2);
  RotTrans((SVECTOR *)objectPosition, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)objectRotation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
