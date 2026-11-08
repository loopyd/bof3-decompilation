#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map.
 * The cell is not volatile here: this body reads it once, before any store goes
 * through the loaded pointer, so the original materializes `lui $a1,0x1F80` /
 * `lw $a1,0x44($a1)` exactly once and every later access is a field of that
 * pointer. Declaration style follows the exact same-target siblings
 * dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic05903_801EF224 and
 * incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvoke
 * HelperWhenZeroBmagicMagic05903_801EF5C0. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay's image. This
 * target's generated undefined-syms map already binds the address and the name
 * is its own D_<address> form, so the binding needs no map row. The words 0x34
 * and 0x38 and the halfword 0x3E it supplies are the record fields the exact
 * bmagic siblings copy from this same pointer. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared screen matrix composed under the freshly built work record transform.
 * The target's generated undefined-syms map owns the address through the same
 * D_<address> row, so no map row is needed. Every access is a matrix base
 * handed to MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF638
 * @behavior Pushes the GTE matrix stack, then reads the shared battle record
 *           pointer at 0x801EB4E0 and the work record published at the
 *           scratchpad pointer cell 0x1F800044 once each. It copies the shared
 *           record words 0x34 and 0x38 and halfword 0x3E into the same work
 *           record offsets and fills a three halfword triplet with zeros. It
 *           then reads the copied fields back and builds a second halfword
 *           triplet whose first two elements are work record words 0x34 and
 *           0x38 shifted right by nine and biased by minus 0x4000 and whose
 *           third element is half the negated work record signed halfword 0x3E.
 *           RotTrans projects that triplet into the translation field of a
 *           0x20-byte scratch matrix while storing a flag word beside it,
 *           RotMatrix turns the zeroed triplet into that matrix, MulMatrix2
 *           composes the shared matrix at 0x801492E8 under it, and the result is
 *           published through SetRotMatrix and SetTransMatrix. Takes no
 *           arguments, returns nothing, and the 0x50-byte frame keeps $ra and
 *           $s0 across the two matrix calls; the caller pairs it with a
 *           PopMatrix. Bytes 0x801EF638..0x801EF70B are not identical to the
 *           near sibling projectWorkRecordPositionAndInstallTransform at
 *           0x801EF52C in bmagic magic106: 46 of its 50 words differ, first at
 *           0x801EF64A, and it takes its two triplets from another record and
 *           the opposite way round, so that sibling is shape evidence only and
 *           this source was seeded from the original bytes, not cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyBattleRecordPositionPairAndHalfwordThenProjectWorkRecordAndInstallTransformBmagicMagic05903_801EF638(
    void) {
  u8 *record;
  u8 *work;
  s16 rotation[3];
  s16 translation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();

  record = D_801EB4E0;
  work = g_battle_work;
  translation[0] = 0;
  translation[1] = 0;
  translation[2] = 0;
  *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
  *(u16 *)(work + 0x3E) = *(u16 *)(record + 0x3E);
  rotation[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  rotation[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  rotation[2] = (s16)(-*(s16 *)(work + 0x3E) / 2);
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
