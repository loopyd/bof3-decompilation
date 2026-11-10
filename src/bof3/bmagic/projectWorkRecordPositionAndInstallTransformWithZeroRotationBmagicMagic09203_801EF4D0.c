#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell 0x1F800044, bound by this target's own
 * map row g_battle_work. The original reads it once, before any call, so it
 * materializes a single `lui $v1,0x1F80` / `lw $v1,0x44($v1)` pair and every
 * later access is a field of the loaded pointer. Declaration matches the exact
 * same-target sibling
 * dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkBytes0And2SetBmagicMagic09203_801EF044,
 * which reads the same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built transform. This
 * target's generated undefined_syms_auto.txt row owns the address, so no
 * target-map row is needed. The only access is a matrix base handed to
 * MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF4D0
 * @behavior Pushes the GTE matrix stack and reads the work-record pointer cell
 *           at 0x1F800044 once. It fills a translation halfword triplet with
 *           zeros, then builds the work record's position triplet from the
 *           pointed-to record whose first two elements are that record's words
 *           0x34 and 0x38 shifted right by nine and biased by minus 0x4000 and
 *           whose third element is half the negated signed halfword 0x3E.
 *           RotTrans projects that position through the matrices the caller
 *           installed into the translation field of a 0x20-byte scratch matrix
 *           while storing a flag word beside it, RotMatrix turns the zeroed
 *           triplet into an identity rotation in that same scratch matrix,
 *           MulMatrix2 composes the shared matrix at 0x801492E8 under it, and
 *           the result is published through SetRotMatrix and SetTransMatrix.
 *           Takes no arguments, returns nothing, and the 0x50-byte frame keeps
 *           $ra at 0x4C and $s0 at 0x48 across the matrix calls; the caller
 *           pairs it with a PopMatrix. Bytes 0x801EF4D0..0x801EF57F are
 *           byte-identical to the exact sibling
 *           projectWorkObjectPositionAndInstallTransformWithZeroTranslation at
 *           0x801F06E0 in bmagic magic006, so `lift clone` proved the range
 *           duplicate before this source was written; the sibling keeps the
 *           same three record reads and the same zeroed triplet, and this file
 *           is self-contained, owning no shared declaration with it. The record
 *           fields stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformWithZeroRotationBmagicMagic09203_801EF4D0(
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
