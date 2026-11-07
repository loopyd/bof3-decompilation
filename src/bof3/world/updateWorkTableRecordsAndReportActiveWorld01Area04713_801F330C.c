#include "bof3/bof3.h"

extern u32 D_80145EC4[]; /* @source 0x80145EC4 @kind table */

/* Per-record update helper of this payload at 0x801F2F60: reads the record's
 * signed phase halfword at +0x10 and its halfword amplitude at +0x12, adds the
 * amplitude while its own counter is zero and subtracts it otherwise, and uses
 * the result as the offset it draws the record with. It takes the record
 * pointer in $a0 and its result is unused, so the call site agrees with a void
 * prototype. */
void func_801F2F60(u8* record);

/* @source 0x801F330C
 * @behavior Overlay per-frame work-table step: walks the 0x10 records of the
 *           0x18-byte-stride work table at 0x800E4800 from the state byte at
 *           0x800E4814, the main-RAM work area this overlay shares with the
 *           world and scenario overlays. A record whose state byte +0x14 reads
 *           zero is skipped; any other record marks the frame active, whether
 *           or not it is retired in the same pass. A record still in sub-phase
 *           0 is armed: halfword +0x10 = 0, halfword +0x12 = 0x10, bytes +0x17
 *           and +0x16 = 0x20, and the sub-phase byte +0x15 is incremented to 1.
 *           A record in sub-phase 1 copies the shared three-word position
 *           descriptor D_80145EC4, D_80145EC8 and D_80145ECC + 0x800000 into
 *           its words +0x00, +0x04 and +0x08, advances the phase halfword at
 *           +0x10 by 8, decrements the 0x20 countdown at +0x17, clears the
 *           state byte +0x14 once the +0x16 countdown reaches zero and calls
 *           func_801F2F60 with the record pointer. It returns its low byte set
 *           to 1 when at least one record was active this frame and 0 when none
 *           was, takes no arguments and uses a 0x30-byte frame holding s0-s5
 *           and the saved return address.
 *
 * Shape notes from the original bytes: the state byte it walks is the one the
 * exact lift clearWorkTableByte14At800E4800World01Area04713_801F3298 clears, so
 * the table base, stride, record count and byte counter type come from that
 * sibling, and its 0x800E4814 lease is the same one the exact
 * seedThreeRotatingQuadRecordsWorld02Area10313_801F349C lift walks. The frame
 * word flag is a byte: only a byte-typed flag keeps the arm/advance switch
 * comparison materialising its own 1 (`addiu $v0, $zero, 0x1` at 0x801F3370)
 * beside the byte store of the same value, which is why the report value is
 * returned as a low byte and why the caller masks it. Its only two callers are
 * func_801F2CC8, which ignores the result, and func_801F2D44, whose
 * `andi $v0, $v0, 0xFF` / `bnez` after the `jal` at 0x801F2D88 proves the
 * reporting call site consumes that low byte and skips func_80196070 when it is
 * zero. The three-word descriptor it copies is the one func_801F31F4 publishes
 * to func_801F2DB4 one instruction run earlier, so it stays address-anchored at
 * its shared main-RAM binding.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 updateWorkTableRecordsAndReportActiveWorld01Area04713_801F330C(void) {
  u8 active;
  u8* record;
  u8 i;

  active = 0;
  record = PSX_PTR(u8, 0x800E4800u);
  i = 0;
  do {
    if (FIELD_REF(u8, record, 0x14) != 0) {
      active = 1;
      switch (FIELD_REF(u8, record, 0x15)) {
        case 0:
          FIELD_REF(u16, record, 0x10) = 0;
          FIELD_REF(u16, record, 0x12) = 0x10;
          FIELD_REF(u8, record, 0x17) = 0x20;
          FIELD_REF(u8, record, 0x16) = 0x20;
          FIELD_REF(u8, record, 0x15) += 1;
          break;
        case 1:
          FIELD_REF(u32, record, 0x00) = D_80145EC4[0];
          FIELD_REF(u32, record, 0x04) = D_80145EC4[1];
          FIELD_REF(u32, record, 0x08) = D_80145EC4[2] + 0x800000u;
          FIELD_REF(u16, record, 0x10) += 8;
          FIELD_REF(u8, record, 0x17) -= 1;
          if (--FIELD_REF(u8, record, 0x16) == 0) {
            FIELD_REF(u8, record, 0x14) = 0;
          }
          func_801F2F60(record);
          break;
      }
    }
    record += 0x18;
    i += 1;
  } while (i < 0x10);
  return active;
}
