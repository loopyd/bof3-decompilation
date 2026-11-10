#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row. The original materializes the cell with one lui/lw pair after
 * PushMatrix and never re-reads it, so the plain named cell suffices and the
 * plain `u8 *` view carries every field this body touches. Declaration style
 * follows the exact same-overlay siblings
 * countDownWorkByte9ThenSeedAngleFromByteBAddHalvedRotatorStepsToWords34And38CopyWord3CSetWord14ToRandPlus128Word20ToMinus12ResetByte9To16AndAdvanceWorkByte2BmagicMagic01303_801EFE80
 * and reserveFirstFreeRecordBmagicMagic01303_801F0358. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built work-object transform.
 * This target's generated undefined_syms_auto.txt owns the address through the
 * row D_801492E8, so no target-map row is needed. The only access is the matrix
 * base handed to MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801F00A8
 * @behavior Pushes the GTE matrix stack and reads the scratchpad work-object
 *           pointer cell 0x1F800044 once. It fills a halfword triplet whose
 *           first two elements are zero and whose third element is the work
 *           object byte 0x9 shifted left by six, the countdown the exact sibling
 *           countDownWorkByte9ThenSeedAngleFromByteBAddHalvedRotatorStepsToWords34And38CopyWord3CSetWord14ToRandPlus128Word20ToMinus12ResetByte9To16AndAdvanceWorkByte2BmagicMagic01303_801EFE80
 *           decrements each step and re-arms to 0x10, so the triplet sweeps a
 *           quarter turn down to zero in 64-unit steps. It then builds a second
 *           halfword triplet from the work object itself: words 0x34 and 0x38
 *           shifted right by nine and biased by minus 0x4000, and half the
 *           negated signed halfword 0x3E. RotTrans projects that triplet into
 *           the translation field of a 0x20-byte scratch matrix while storing a
 *           flag word beside it, RotMatrix turns the zero-and-angle triplet into
 *           that matrix, MulMatrix2 composes the shared matrix at 0x801492E8
 *           under it, and the result is published through SetRotMatrix and
 *           SetTransMatrix. Takes no arguments, returns nothing, and the
 *           0x50-byte frame keeps $ra at 0x4C and $s0 at 0x48 across the two
 *           matrix calls; the caller at 0x801EFDF8 pairs it with PopMatrix
 *           behind work bytes 0 and 9 both being set. Bytes
 *           0x801F00A8..0x801F0163 differ from the near sibling
 *           projectWorkObjectPositionAndInstallTransformWithWorkByteAAngle at
 *           0x801EF654 in bmagic magic094 in exactly two words - the lbu offset
 *           (0x9 here, 0xA there) and the shift count (6 here, 4 there) - and
 *           agree in all 45 others, so that sibling supplied statement order,
 *           operand forms and declaration style while these bytes supply the
 *           loaded offset and the shift count. The record fields stay
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkObjectPositionAndInstallTransformWithWorkByte9AngleBmagicMagic01303_801F00A8(
    void) {
  u8 *work;
  s16 objectPosition[3];
  s16 objectRotation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();

  work = g_battle_work;
  objectRotation[0] = 0;
  objectRotation[1] = 0;
  objectRotation[2] = work[0x9] << 6;
  objectPosition[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  objectPosition[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  objectPosition[2] = (s16)(-*(s16 *)(work + 0x3E) / 2);
  RotTrans((SVECTOR *)objectPosition, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)objectRotation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
