#include "bof3/bof3.h"

/* Work-record pointer cell at 0x801F1054, in this overlay's trailing data
 * block. The frame loop at 0x801EECAC publishes the selected record into the
 * cell with one `sw`, the dispatcher at 0x801EF340 selects a handler through
 * the record byte 0x01, and every handler of this overlay reads the record back
 * through the cell. Splat binds the address through this target's generated
 * undefined_syms_auto.txt row, so no map row is needed. The original reloads
 * the cell after a byte store through the loaded pointer, so the references
 * that follow a store name the cell again. */
extern u8 *D_801F1054; /* @source 0x801F1054 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt
 * row, so no map row is needed. Both accesses read a word field, so the local
 * declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8, the pair the neighbouring
 * step handler at 0x801F0220 also hands a record angle to. Both addresses keep
 * their generated undefined_funcs_auto.txt rows. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801EF704
 * @behavior Entry 4 of the six-word handler run at 0x801F0404, called with no
 * arguments by the dispatcher at 0x801EF340. It advances the work-record
 * halfword angle at offset 0x0E by 0x40 within 0x1000, stores it back and hands
 * it to the shared rotator at 0x801782FC. It then re-reads the pointer cell and
 * stores the shared battle word at 0x801EB4E0 offset 0x34 plus that rotator
 * result multiplied by the record signed halfword 0x0C into the record word at
 * offset 0x10; the second rotator at 0x801783C8 receives the record signed
 * halfword 0x0E. The third cell read caches the shared battle word at offset
 * 0x38, increments the record byte 0x0B and stores the cached word plus the
 * second rotator result times halfword 0x0C into the record word at offset
 * 0x14. The fourth cell read is taken before that store and feeds the first
 * byte of the ramp tail, while the later ramp reads name the cell again. The
 * tail ramps four record bytes: byte 0x05 rises while it stays below 0x0C,
 * bytes 0x06 and 0x07 fall while they stay at 5 or more, and byte 0x03 rises
 * while it stays below 7. Finally byte 0x08 is decremented and its
 * truncated result tested, and when that countdown reaches zero the record
 * byte 0x02 is incremented through a fresh cell read. The 0x17C-byte body runs
 * in a 0x18-byte frame that keeps only $ra across the two rotator calls, and
 * takes no arguments and returns nothing. The rotation half is
 * instruction-for-instruction identical to the exact lift
 * stepWorkRecordAngleProjectPositionPairAndCountDownByte9OnFrameParityThenClear
 * RecordAtZeroBmagicMagic10603_801EF418, whose target keeps it at 0x801EF418,
 * while the ramp and countdown tail differs, so that relocation-different
 * sibling is shape evidence only and this source was seeded from the original
 * bytes, not cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkRecordAngleProjectPositionPairIncrementWorkByteBAndRampWorkBytes3And5Through7ThenAdvanceWorkByte2OnZeroWorkByte8BmagicMagic06303_801EF704(void) {
  u8 *record;
  u8 *record2;
  u8 *record3;
  u8 *record4;
  s32 angle;
  s32 step;
  u32 position;

  record = D_801F1054;
  angle = (FIELD_REF(u16, record, 0x0E) + 0x40) & 0xFFF;
  FIELD_REF(u16, record, 0x0E) = angle;
  step = func_801782FC(angle);
  record2 = D_801F1054;
  FIELD_REF(u32, record2, 0x10) =
      FIELD_REF(u32, D_801EB4E0, 0x34) +
      step * (s32)FIELD_REF(s16, record2, 0x0C);
  step = func_801783C8((s32)FIELD_REF(s16, record2, 0x0E));
  record3 = D_801F1054;
  position = FIELD_REF(u32, D_801EB4E0, 0x38);
  FIELD_REF(u8, record3, 0x0B)++;
  record4 = D_801F1054;
  FIELD_REF(u32, record3, 0x14) =
      position + step * (s32)FIELD_REF(s16, record3, 0x0C);
  if (FIELD_REF(u8, record4, 0x05) < 0x0C) {
    FIELD_REF(u8, record4, 0x05)++;
  }
  if (FIELD_REF(u8, D_801F1054, 0x06) >= 5) {
    FIELD_REF(u8, D_801F1054, 0x06)--;
  }
  if (FIELD_REF(u8, D_801F1054, 0x07) >= 5) {
    FIELD_REF(u8, D_801F1054, 0x07)--;
  }
  if (FIELD_REF(u8, D_801F1054, 0x03) < 7) {
    FIELD_REF(u8, D_801F1054, 0x03)++;
  }
  if (--FIELD_REF(u8, D_801F1054, 0x08) == 0) {
    FIELD_REF(u8, D_801F1054, 0x02)++;
  }
}
