#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell 0x1F800044, bound by this target's own
 * map row g_battle_work. This body reads it once, before any call, so the
 * original materializes a single `lui $v1,0x1F80` / `lw $v1,0x44($v1)` pair and
 * every later access is a field of the loaded pointer. Declaration matches the
 * exact bmagic sibling
 * projectWorkRecordPositionAndInstallTransformWithZeroRotation, which reads the
 * same cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay's image. This
 * target's generated undefined-syms map already binds the address under its own
 * D_<address> form, so the binding needs no map row. The halfword 0x3E it
 * supplies here is the record field the exact bmagic siblings copy into the work
 * record before projecting it. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared screen matrix composed under the freshly built work record transform.
 * The target's generated undefined-syms map owns the address through the same
 * D_<address> row, so no map row is needed. Every access is a matrix base handed
 * to MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EFB90
 * @behavior Pushes the GTE matrix stack and reads the work record published at
 *           the scratchpad pointer cell 0x1F800044 once. It fills a translation
 *           halfword triplet with zeros, writing its highest element first,
 *           then builds a rotation triplet whose
 *           first two elements are work record words 0x34 and 0x38 shifted right
 *           by nine and biased by minus 0x4000 and whose third element is half
 *           the negated signed halfword 0x3E of the shared battle record read
 *           through the main-RAM pointer cell 0x801EB4E0. RotTrans projects that
 *           triplet into the translation field of a 0x20-byte scratch matrix
 *           while storing a flag word beside it, RotMatrix turns the zeroed
 *           triplet into an identity rotation in that same matrix, MulMatrix2
 *           composes the shared matrix at 0x801492E8 under it, and the result is
 *           published through SetRotMatrix and SetTransMatrix. Takes no
 *           arguments, returns nothing, and the 0x50-byte frame keeps $ra at
 *           0x4C and $s0 at 0x48 across the matrix calls; the caller pairs it
 *           with a PopMatrix. Bytes 0x801EFB90..0x801EFC47 are not identical to
 *           the near sibling
 *           projectWorkRecordPositionAndInstallTransformWithZeroRotation at
 *           0x801EF4D0 in bmagic magic092: that body is two instructions shorter
 *           because it takes its third rotation element from its own work record
 *           instead of from the shared battle record, so it is shape evidence
 *           only and this source was seeded from the original bytes. The record
 *           fields stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformWithSharedRecordHalfword3EBmagicMagic04603_801EFB90(
    void) {
  u8 *work;
  u8 *record;
  s16 rotation[3];
  s16 translation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();

  work = g_battle_work;
  translation[2] = 0;
  translation[1] = 0;
  translation[0] = 0;
  rotation[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  rotation[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  record = D_801EB4E0;
  rotation[2] = (s16)(-*(s16 *)(record + 0x3E) / 2);
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
