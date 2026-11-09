#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row. The original materializes the cell with one lui/lw pair after PushMatrix
 * and never re-reads it, so the plain named cell suffices. Declaration style
 * follows the exact same-target siblings
 * copyRecordHalfwordPairIntoWorkThenOnWorkByteBFFSubmitBattlerRecordForwardBattleByteRaiseFlag4AndInvokeHelperBmagicMagic09403_801EED60
 * and incrementWorkByte9ThenSetGlobalWorkByteBToFFAndInvokeHelperWhenWorkByte9Is20BmagicMagic09403_801EF0FC. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built work-object transform.
 * This target's generated undefined_syms_auto.txt owns the address through the
 * row D_801492E8, so no target-map row is needed. The only access is the matrix
 * base handed to MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF654
 * @behavior Pushes the GTE matrix stack and reads the scratchpad work-object
 *           pointer cell 0x1F800044 once. It fills a halfword triplet whose
 *           first two elements are zero and whose third element is the work
 *           object byte 0xA shifted left by four, the per-frame ramp the exact
 *           sibling incrementWorkByteAThenIncrementWorkByte9AndSubmitSelectedBattlerRecordEffectWhenByte9Is10ClearByte9AndAdvanceWorkByte1BmagicMagic09403_801EEF7C
 *           advances. It then builds a second halfword triplet from the work
 *           object itself: words 0x34 and 0x38 shifted right by nine and biased
 *           by minus 0x4000, and half the negated signed halfword 0x3E.
 *           RotTrans projects that triplet into the translation field of a
 *           0x20-byte scratch matrix while storing a flag word beside it,
 *           RotMatrix turns the zero-and-ramp triplet into that matrix,
 *           MulMatrix2 composes the shared matrix at 0x801492E8 under it, and
 *           the result is published through SetRotMatrix and SetTransMatrix.
 *           Takes no arguments, returns nothing, and the 0x50-byte frame keeps
 *           $ra and $s0 across the two matrix calls; the caller pairs it with a
 *           PopMatrix. Bytes 0x801EF654 through 0x801EF70F are not identical to
 *           the exact sibling projectWorkObjectPositionAndInstallTransformWithZeroTranslation
 *           at 0x801F06E0 in bmagic magic006, which stores a literal zero where
 *           this span loads and shifts work byte 0xA, so that sibling supplied
 *           statement order and operand forms only and this source writes its
 *           own expression from these bytes rather than cloning it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkObjectPositionAndInstallTransformWithWorkByteAAngleBmagicMagic09403_801EF654(
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
  objectRotation[2] = work[0xA] << 4;
  objectPosition[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  objectPosition[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  objectPosition[2] = (s16)(-*(s16 *)(work + 0x3E) / 2);
  RotTrans((SVECTOR *)objectPosition, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)objectRotation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
