#include "bof3/bof3.h"

/* Work-record pointer cell at the tail of this overlay's data block. The
 * dispatcher at 0x801EF340 reads the same cell, and the record it points into
 * is one of the 0x20-byte entries of the pool at 0x801F0454, so the cell always
 * holds a live record when this handler runs. Splat binds the address through
 * this target's generated undefined_syms_auto.txt row, so no map row is
 * needed. */
extern u8 *D_801F1054; /* @source 0x801F1054 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the generated undefined_syms_auto.txt row, so no
 * map row is needed; both accesses here read a word field, so the declaration
 * keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the samples of a 12-bit
 * turn in the fixed-point scale the products elsewhere divide out, with the
 * 0x801783C8 sample feeding x halfwords and the 0x801782FC sample y halfwords.
 * Both addresses keep their generated undefined_funcs_auto.txt rows. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801EFF18
 * @behavior Step handler of the bmagic effect record published through the
 * pointer cell at 0x801F1054. It adds 0x20 to the record 12-bit halfword angle
 * at offset 0x0E inside 0x1000, stores that sum back, hands it to the shared
 * rotator at 0x801782FC, and stores the shared battle word at D_801EB4E0 offset
 * 0x34 plus that rotator result shifted left five into the record word at
 * offset 0x10. The second shared rotator at 0x801783C8 receives the record
 * signed halfword at offset 0x0E, and its result shifted left five is added to
 * the shared battle word at offset 0x38 and stored in the record word at offset
 * 0x14. It then adds two to the record byte at offset 0x09, adds one to the
 * record byte at offset 0x0B, and when the byte at offset 0x09 totals 0x10 it
 * advances the record step byte at offset 0x02. The original re-materialises
 * each pointer-cell read across the two calls and the byte stores, so the cell
 * is loaded five times and the shared battle pointer twice; the read that feeds
 * the byte 0x0B update is issued between the byte 0x09 store and the record word
 * at offset 0x14, which is why that word takes the pointer read prior to it. The
 * 0xD0-byte body runs in a 0x18-byte frame that keeps only $ra, takes no
 * arguments and returns nothing. The neighbouring bodies at 0x801EFE04 and
 * 0x801EFFE8 carry the same angle advance and position-pair projection with
 * different tails, so they are shape evidence only and not clone proof.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkRecordAngleProjectPositionPairIncrementWorkByte9ByTwoAndWorkByteBThenAdvanceWorkByte2AtSixteenBmagicMagic06303_801EFF18(void) {
  u8 *record;
  u8 *record2;
  u8 *record3;
  u8 *record4;
  u8 *record5;
  s32 angle;
  s32 step;
  u32 position;

  record = D_801F1054;
  angle = (FIELD_REF(u16, record, 0x0E) + 0x20) & 0xFFF;
  FIELD_REF(u16, record, 0x0E) = angle;
  step = func_801782FC(angle);
  record2 = D_801F1054;
  FIELD_REF(u32, record2, 0x10) =
      FIELD_REF(u32, D_801EB4E0, 0x34) + (step << 5);
  step = func_801783C8((s32)FIELD_REF(s16, record2, 0x0E));
  record3 = D_801F1054;
  position = FIELD_REF(u32, D_801EB4E0, 0x38);
  FIELD_REF(u8, record3, 0x09) += 2;
  record4 = D_801F1054;
  FIELD_REF(u32, record3, 0x14) = position + (step << 5);
  FIELD_REF(u8, record4, 0x0B)++;
  record5 = D_801F1054;
  if (FIELD_REF(u8, record5, 0x09) == 0x10) {
    FIELD_REF(u8, record5, 0x02)++;
  }
}
