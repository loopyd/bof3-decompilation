#include "bof3/bof3.h"

/* Work-record pointer cell at the tail of this overlay's data block, the same
 * cell the dispatcher at 0x801EF340, the clear at 0x801F03B0 and the step
 * handler at 0x801EFF18 read. Splat binds the address through this target's
 * generated undefined_syms_auto.txt row, so no map row is needed. The original
 * re-materialises the cell for every access because the byte stores through the
 * loaded pointer may alias the cell itself. */
extern u8 *D_801F1054; /* @source 0x801F1054 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt
 * row, so no map row is needed. The two position reads take word fields, so the
 * local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8, the same pair the step
 * handler at 0x801EFF18 hands the record angle to. Both addresses keep their
 * generated undefined_funcs_auto.txt rows. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Clears work-record bytes 0x00 through 0x04 of the record published through
 * the cell at 0x801F1054; defined in this overlay at 0x801F03B0. */
void clearRecordBytes0Through4BmagicMagic06303_801F03B0(void);

/* @source 0x801F0220
 * @behavior Step handler of the bmagic effect record published through the
 * pointer cell at 0x801F1054. It reads the shared battle work byte 0x02 through
 * 0x801EB4E0 and branches on that byte equalling three: when it does it adds
 * two to the record byte 0x0B and 0x40 to the record halfword angle at offset
 * 0x0E, otherwise it adds one to the record byte 0x0B and 0x20 to the same
 * angle. Both arms end by masking the sum to twelve bits and storing it back,
 * and the masked store reuses the record pointer the arm loaded for the angle
 * read, so the cell is read twice per arm. The masked halfword is then re-read
 * as a signed halfword and handed to the shared rotator at 0x801782FC; the
 * rotator result shifted left five is added to the shared battle word at offset
 * 0x34 and stored in the record word at offset 0x10. The second shared rotator
 * at 0x801783C8 receives the record signed halfword at offset 0x0E again, and
 * its result shifted left five is added to the shared battle word at offset
 * 0x38 and stored in the record word at offset 0x14, the store filling the
 * second call delay slot. It then decrements the record byte at offset 0x09 and
 * tests the truncated result: when that countdown reaches zero it decrements
 * the shared battle byte 0x0B and invokes the record clear at 0x801F03B0, the
 * store filling that call delay slot. The 0x134-byte body runs in a 0x18-byte
 * frame that keeps only $ra, takes no arguments and returns nothing. The
 * neighbouring still-asm body at 0x801F00F0 differs in fourteen words: the
 * literal it adds to the record byte 0x0B, the record offset it counts down
 * (0x0A there, 0x09 here), its step-byte tail, and the absolute words those
 * length changes shift. It is shape evidence only, so this source was seeded
 * from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkRecordAngleProjectPositionPairAddSixtyFourToAngleAndTwoToWorkByteBWhenGlobalWorkByte2IsThreeElseThirtyTwoAndOneThenCountDownWorkByte9DecrementGlobalWorkByteBAndClearRecordOnZeroBmagicMagic06303_801F0220(void) {
  u8 *record;
  u8 *record2;
  u8 *record3;
  u8 *record4;
  u8 *record5;
  s32 angle;
  s32 step;
  u32 position;

  if (FIELD_REF(u8, D_801EB4E0, 0x02) == 3) {
    record = D_801F1054;
    FIELD_REF(u8, record, 0x0B) += 2;
    record2 = D_801F1054;
    angle = FIELD_REF(u16, record2, 0x0E) + 0x40;
  } else {
    record = D_801F1054;
    FIELD_REF(u8, record, 0x0B)++;
    record2 = D_801F1054;
    angle = FIELD_REF(u16, record2, 0x0E) + 0x20;
  }
  angle &= 0xFFF;
  FIELD_REF(u16, record2, 0x0E) = angle;
  record3 = D_801F1054;
  step = func_801782FC((s32)FIELD_REF(s16, record3, 0x0E));
  record4 = D_801F1054;
  FIELD_REF(u32, record4, 0x10) =
      FIELD_REF(u32, D_801EB4E0, 0x34) + (step << 5);
  step = func_801783C8((s32)FIELD_REF(s16, record4, 0x0E));
  record5 = D_801F1054;
  position = FIELD_REF(u32, D_801EB4E0, 0x38);
  FIELD_REF(u32, record5, 0x14) = position + (step << 5);
  if (--FIELD_REF(u8, record5, 0x09) == 0) {
    FIELD_REF(u8, D_801EB4E0, 0x0B)--;
    clearRecordBytes0Through4BmagicMagic06303_801F03B0();
  }
}
