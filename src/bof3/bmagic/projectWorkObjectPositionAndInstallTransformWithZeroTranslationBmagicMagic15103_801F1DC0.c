#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and shared by this overlay's dispatchers and handlers. The body reads it once,
 * before any call, so the original materializes `lui $v1,0x1F80` /
 * `lw $v1,0x44($v1)` exactly once and every later access is a field of the
 * loaded pointer. Declaration style follows the exact same-target siblings
 * projectWorkObjectPositionAndInstallTransformWithZeroTranslation at 0x801F01BC
 * and decrementWorkTintChannel5DThenIncrementWorkByte2WhenChannelIs8 at
 * 0x801F00E8, which bind the identical cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built transform. This
 * target's generated undefined_syms_auto.txt row D_801492E8 = 0x801492E8 owns
 * the address, so no target-map row is needed. The only access is a matrix base
 * handed to MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801F1DC0
 * @behavior Pushes the GTE matrix stack and reads the work-object pointer cell
 *           at 0x1F800044 once. It fills a three halfword angle triplet with
 *           zeros, then builds a rotation triplet from the pointed-to work
 *           object whose first two elements are that object's words 0x34 and
 *           0x38 shifted right by nine and biased by minus 0x4000 and whose
 *           third element is half the negated signed halfword 0x3E. RotTrans
 *           projects the rotation triplet into the translation field of a
 *           0x20-byte scratch matrix while storing its flag word beside it,
 *           RotMatrix turns the zeroed triplet into that matrix, MulMatrix2
 *           composes the shared matrix at 0x801492E8 under it, and the result
 *           is published through SetRotMatrix and SetTransMatrix. Takes no
 *           arguments, returns nothing, and the 0x50-byte frame keeps $ra and
 *           $s0 across the matrix calls; the caller pairs it with a PopMatrix.
 *           Bytes 0x801F1DC0 through 0x801F1E6F are byte-identical to the exact
 *           same-target sibling projectWorkObjectPositionAndInstallTransform
 *           WithZeroTranslationBmagicMagic15103_801F01BC - a 176-byte cmp of
 *           both spans at their own (runtime address - load address) offsets
 *           found no differing byte - so this body carries that sibling's
 *           proven shape, statement order and cell declaration. The
 *           cross-target lift clone adapter declines a same-target span, so the
 *           equality was proven by that payload cmp and the source was seeded
 *           from the twin.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkObjectPositionAndInstallTransformWithZeroTranslationBmagicMagic15103_801F1DC0(
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
