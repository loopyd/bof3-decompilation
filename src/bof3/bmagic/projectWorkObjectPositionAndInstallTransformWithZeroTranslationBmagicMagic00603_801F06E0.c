#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map as
 * g_battle_work. This body reads it once, before any call, so the original
 * materializes `lui $v1,0x1F80` / `lw $v1,0x44($v1)` exactly once and every
 * later access is a field of the loaded pointer. Declaration style follows the
 * exact same-target siblings copyWorkWordTripletClearByte9ThenAdvanceWorkByte2
 * at 0x801F05A4 and incrementWorkByte2WhenGlobalWorkByteBIs1 at 0x801F0644. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built transform. This
 * target's generated undefined_syms_auto.txt row owns the address, so no
 * target-map row is needed. The only access is a matrix base handed to
 * MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801F06E0
 * @behavior Pushes the GTE matrix stack and reads the work-object pointer cell
 *           at 0x1F800044 once. It fills a three halfword translation triplet
 *           with zeros, then builds a rotation triplet from the pointed-to work
 *           object whose first two elements are that object's words 0x34 and
 *           0x38 shifted right by nine and biased by minus 0x4000 and whose
 *           third element is half the negated signed halfword 0x3E. RotTrans
 *           projects the rotation triplet into the translation field of a
 *           0x20-byte scratch matrix while storing a flag word beside it,
 *           RotMatrix turns the zeroed translation triplet into that matrix,
 *           MulMatrix2 composes the shared matrix at 0x801492E8 under it, and
 *           the result is published through SetRotMatrix and SetTransMatrix.
 *           Takes no arguments, returns nothing, and the 0x50-byte frame keeps
 *           $ra and $s0 across the two matrix calls; the caller pairs it with a
 *           PopMatrix. Bytes 0x801F06E0..0x801F078F are not identical to the
 *           near sibling copyBattleRecordPositionPairAndHalfwordThenProject
 *           WorkRecordAndInstallTransform at 0x801EF638 in bmagic magic059,
 *           which copies a record triplet into the work object before reading
 *           the same three fields back, so that sibling is shape evidence only
 *           and this source was seeded from the original bytes, not cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkObjectPositionAndInstallTransformWithZeroTranslationBmagicMagic00603_801F06E0(
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
