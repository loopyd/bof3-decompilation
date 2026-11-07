#include "bof3/bof3.h"

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. The three fields this body reads are the record word
 * x at row offset 0x34 (0x80143FFC), the record word y at row offset 0x38
 * (0x80144000) and the record halfword at row offset 0x3E (0x80144006). The
 * original re-materialises `lui at,0x8014` and adds the scaled row offset for
 * every access, so each field base stays its own address-derived symbol.
 * @source 0x80143FFC @kind unknown
 */
extern u8 D_80143FFC[];
/* @source 0x80144000 @kind unknown */
extern u8 D_80144000[];
/* @source 0x80144006 @kind unknown */
extern u8 D_80144006[];

/* Work-record table of the shared main-executable data segment: 0x140-byte
 * records based at 0x80145E90, counted by the byte at 0x80146254. This overlay
 * passes those records to the shared position gate below without reading a field
 * of its own, so the record stays a byte view here.
 * @source 0x80145E90 @kind table
 */
extern u8 D_80145E90[];
/* @source 0x80146254 @kind unknown */
extern u8 D_80146254;

/* Shared position gate of the main executable, outside this payload: it reports
 * one when the supplied point sits inside the work record's range. The original
 * zero-extends the result byte before testing it, so this overlay sees the
 * unsigned return type. */
u8 func_801BDD58(s32 x, s32 y, s16 reference, s32 range, void* work);

/* @source 0x801CEA10
 * @behavior Tests one front-end record's stored position against the work
 *           records of the shared main-executable data segment. The argument is
 *           the record slot: its 0x74-byte row offset selects the row word x at
 *           0x80143FFC, the row word y at 0x80144000 and the row halfword at
 *           0x80144006, the halfword entering the reference as a signed
 *           sixteen-bit value biased by 0x200. Starting at work record 1 and
 *           while the counter stays below the record count byte D_80146254, it
 *           passes that position triple, the range 1 and the 0x140-byte work
 *           record to the shared gate func_801BDD58 and returns 1 as soon as
 *           that gate reports a match. Otherwise, and whenever the count byte is
 *           not above 1, it returns 0. Work record 0 is never tested. The
 *           original computes the row offset once before the loop and keeps it
 *           in a saved register while each field access rebuilds its own symbol
 *           base, and it walks the work records with a second saved register
 *           that starts at 0x140 and grows by 0x140 in the loop latch's delay
 *           slot; it computes the work-record address first in the loop body but
 *           stores it into the argument slot after the range constant. The
 *           0x28-byte frame only keeps $ra at 0x24 and the three saved registers
 *           across the call. Neither this function's address nor any pointer to
 *           it appears in any word of this payload, so it is an overlay entry
 *           reached from outside the payload and its prologue and return paths
 *           alone place the boundary. Its 0xD0-byte body is byte-identical to
 *           func_801CF738 in the plp025 overlay, whose lifted source names it,
 *           and to func_801CEA10 in the plp257 overlay, which no source claims.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 testFrontEndRecordPositionAgainstWorkRecordsPlcharPlp27a00_801CEA10(u8 slot) {
  s32 i;
  u32 row;
  void* work;

  for (i = 1; i < D_80146254; i++) {
    row = (u32)slot * 0x74u;
    work = &D_80145E90[i * 0x140];
    if (func_801BDD58(*(s32*)(D_80143FFC + row), *(s32*)(D_80144000 + row),
                      (s16)(*(u16*)(D_80144006 + row) + 0x200), 1, work) != 0) {
      return 1;
    }
  }
  return 0;
}
