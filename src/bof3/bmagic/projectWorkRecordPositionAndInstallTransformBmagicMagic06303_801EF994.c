#include "bof3/bof3.h"

/* Work-record pointer cell at the tail of this overlay data block. The
 * dispatcher at 0x801EF340 and the handler at 0x801EF384 read the same cell,
 * and the record it points into is one of the 0x20-byte entries of the pool at
 * 0x801F0454, so the cell holds a live record when this function runs. Splat
 * binds the address through this target generated undefined_syms_auto.txt row,
 * so no map row is needed. Declaration matches the same-target sibling
 * stepWorkRecordAngleProjectPositionPairIncrementWorkByte9ByTwoAndWorkByteBThenAdvanceWorkByte2AtSixteenBmagicMagic06303_801EFF18,
 * which reads the same cell. */
extern u8 *D_801F1054; /* @source 0x801F1054 @kind data */

/* Screen matrix the original composes under the freshly built transform. The
 * shared map owns the address through this target generated
 * undefined_syms_auto.txt row, so no target-map row is needed. Every access is a
 * matrix base handed to MulMatrix2, so the local declaration keeps the object
 * view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF994
 * @behavior Pushes the GTE matrix stack and reads the work-record pointer cell
 *           at 0x801F1054 once. It fills a zeroed translation halfword triplet
 *           whose third element is the record byte 0x03 shifted left by seven
 *           minus the sum of the record halfword 0x0E and 0x800, taken within
 *           0x1000, and a rotation halfword triplet whose first two elements are
 *           the record words 0x10 and 0x14 shifted right by nine and biased by
 *           minus 0x4000 and whose third element is half the negated record
 *           signed halfword 0x1A. RotTrans projects the rotation triplet into
 *           the translation field of a 0x20-byte scratch matrix while storing a
 *           flag word beside it, RotMatrix turns the translation triplet into
 *           that matrix, MulMatrix2 composes the shared matrix at 0x801492E8
 *           under it, and the result is published through SetRotMatrix and
 *           SetTransMatrix. Takes no arguments, returns nothing, and the
 *           0x50-byte frame keeps $ra at 0x4C and $s0 at 0x48 across the matrix
 *           calls; the caller at 0x801EF384 pairs it with a PopMatrix. Bytes
 *           0x801EF994 through 0x801EFA5B are identical to the exact bmagic
 *           magic106 sibling projectWorkRecordPositionAndInstallTransform at
 *           0x801EF52C in every word but the low half of the pointer-cell load,
 *           which carries 0x1054 here and 0x5F8 there, so that
 *           relocation-different sibling is shape evidence only and this source
 *           writes its own cell address rather than being cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformBmagicMagic06303_801EF994(void) {
  u8 *record;
  s16 rotation[3];
  s16 translation[3];
  MATRIX object_work;
  s32 offset;
  long flag;

  PushMatrix();

  record = D_801F1054;
  translation[0] = 0;
  translation[1] = 0;
  offset = FIELD_REF(u16, record, 0x0E) + 0x800;
  translation[2] = ((FIELD_REF(u8, record, 0x03) << 7) - offset) & 0xFFF;
  rotation[0] = (s16)(FIELD_REF(s32, record, 0x10) >> 9) - 0x4000;
  rotation[1] = (s16)(FIELD_REF(s32, record, 0x14) >> 9) - 0x4000;
  rotation[2] = (s16)(-FIELD_REF(s16, record, 0x1A) / 2);
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
