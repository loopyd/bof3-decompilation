#include "bof3/bof3.h"

/* Work-record pointer cell. It is the last word of the overlay data run at
 * 0x801EF9C0 and the word just past the 96-record pool at 0x801EF9F8. The
 * original re-materialises the cell address for each of this function's four
 * reads, so the cell is declared volatile, matching the exact bmagic sibling
 * dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic10603_801EEF24. */
extern u8 *volatile D_801F05F8; /* @source 0x801F05F8 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt row
 * D_801EB4E0 = 0x801EB4E0, so no target-map row is needed. Both accesses read a
 * word field, so the local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared frame counter in main RAM, outside this overlay's image. The shared map
 * owns the address (config/targets/shared/symbols.txt) and the sibling
 * frame-parity lift advanceWorkByteAOnFrameParityInvokePointHelperCountdownWork
 * Byte9DecrementWorkByte2AndDrawSpriteBmagicMagic01200_801EF6A4 reads the same
 * word, its low bit as a per-frame phase. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Rotates the halfword angle argument and returns the scaled step word. */
extern s32 func_801782FC(s32 arg0);

/* Rotates the halfword angle argument and returns the scaled step word. */
extern s32 func_801783C8(s32 arg0);

/* Clears bytes 0x0 through 0x4 of the work record, one volatile cell read per
 * store, ending the effect. */
extern void func_801EF970(void);

/* @source 0x801EF418
 * @behavior Advances the work record's halfword angle at offset 0x0E by 0x40
 *           within 0x1000 and passes that value to func_801782FC. It then
 *           re-reads the pointer cell, stores the rotator result multiplied by
 *           the record's signed halfword 0x0C onto the shared battle word at
 *           D_801EB4E0 offset 0x34, and stores that sum in the record word at
 *           offset 0x10. func_801783C8 receives the record's signed halfword
 *           0x0E, and the third pointer-cell read caches the shared battle word
 *           at offset 0x38, adds byte 0x0A onto the record halfword 0x1A and
 *           increments the record byte 0x0B. The original loads the shared
 *           frame counter's low bit into its own value between those byte
 *           updates and the position store, so the counter is captured in a
 *           local before the record word at offset 0x14 takes the cached offset
 *           0x38 word plus the second rotator result times halfword 0x0C. When
 *           that captured bit is set it decrements the record byte 0x09 through
 *           a fourth pointer-cell read and, when the countdown reaches zero,
 *           decrements byte 0x0B of the shared battle word and calls
 *           func_801EF970 to clear the record. Takes no arguments, returns
 *           nothing, and the 0x18-byte frame only keeps $ra across the two
 *           rotator calls and the clear. Bytes 0x801EF418 through 0x801EF52B
 *           differ from the still-unlifted copy at emi/bmagic/magic063/03
 *           address 0x801EF880 in only five words: the four record-cell address
 *           immediates and the clear call target. So that relocation-different
 *           sibling is shape evidence only and the source was seeded from the
 *           original bytes, not cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkRecordAngleProjectPositionPairAndCountDownByte9OnFrameParityThenClearRecordAtZeroBmagicMagic10603_801EF418(void) {
  u8 *record;
  u8 *record2;
  u8 *record3;
  u8 *record4;
  s32 angle;
  s32 step;
  u32 position;
  u32 flag;

  record = D_801F05F8;
  angle = (FIELD_REF(u16, record, 0x0E) + 0x40) & 0xFFF;
  FIELD_REF(u16, record, 0x0E) = angle;
  step = func_801782FC(angle);
  record2 = D_801F05F8;
  FIELD_REF(u32, record2, 0x10) =
      FIELD_REF(u32, D_801EB4E0, 0x34) + step * (s32)FIELD_REF(s16, record2, 0x0C);
  step = func_801783C8((s32)FIELD_REF(s16, record2, 0x0E));
  record3 = D_801F05F8;
  position = FIELD_REF(u32, D_801EB4E0, 0x38);
  FIELD_REF(u16, record3, 0x1A) += FIELD_REF(u8, record3, 0x0A);
  FIELD_REF(u8, record3, 0x0B)++;
  flag = D_80143E6C & 1;
  FIELD_REF(u32, record3, 0x14) =
      position + step * (s32)FIELD_REF(s16, record3, 0x0C);
  if (flag) {
    record4 = D_801F05F8;
    if (--FIELD_REF(u8, record4, 0x09) == 0) {
      FIELD_REF(u8, D_801EB4E0, 0x0B)--;
      func_801EF970();
    }
  }
}
