#include "bof3/bof3.h"

/* Work-record pointer cell. It is the last word of the overlay data run at
 * 0x801EF9C0 and the word just past the 96-record pool at 0x801EF9F8. This
 * function reads the cell once, through the same volatile view the exact
 * bmagic siblings dispatchWorkByte2HandlerThenDrawAndPopWhenActive at
 * 0x801EEF24 and stepWorkRecordAngle at 0x801EF418 use, so the cell is
 * declared volatile here too. */
extern u8 *volatile D_801F05F8; /* @source 0x801F05F8 @kind data */

/* Screen matrix the original composes under the freshly built work record
 * transform. The shared map owns the address through this target's generated
 * undefined_syms_auto.txt row D_801492E8 = 0x801492E8, so no target-map row is
 * needed. Every access is a matrix base handed to MulMatrix2, so the local
 * declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF52C
 * @behavior Pushes the GTE matrix stack and reads the work-record pointer cell
 *           at 0x801F05F8 once. It fills a zeroed translation halfword triplet
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
 *           0x50-byte frame keeps $ra and $s0 across the two matrix calls; the
 *           caller pairs it with a PopMatrix. Bytes 0x801EF52C through
 *           0x801EF5F3 differ from the exact scenario sibling
 *           setupSceneObjectTransform at 0x801D0EE0 by only the source of the
 *           three record reads and the record field offsets, so that
 *           relocation-different sibling is shape evidence only and this source
 *           was seeded from the original bytes, not cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformBmagicMagic10603_801EF52C(void) {
  u8 *record;
  s16 rotation[3];
  s16 translation[3];
  MATRIX object_work;
  s32 offset;
  long flag;

  PushMatrix();

  record = D_801F05F8;
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
