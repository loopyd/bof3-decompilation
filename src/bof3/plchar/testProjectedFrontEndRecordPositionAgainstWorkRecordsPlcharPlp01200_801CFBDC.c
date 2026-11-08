#include "bof3/bof3.h"

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body reads three fields of the row the argument
 * slot selects: the record word x at row offset 0x34 (0x80143FFC), the record
 * word y at row offset 0x38 (0x80144000) and the record halfword at row offset
 * 0x3E (0x80144006), which the original loads with `lh` and hands on as a signed
 * sixteen-bit reference with no bias. Every field base keeps its own
 * address-derived symbol, so each access re-materialises `lui at,0x8014` and adds
 * the scaled row offset.
 * @source 0x80143FFC @kind unknown
 */
extern u8 D_80143FFC[];
/* @source 0x80144000 @kind unknown */
extern u8 D_80144000[];
/* @source 0x80144006 @kind unknown */
extern u8 D_80144006[];

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the active work area's
 * route byte at offset 0x08. The original scales that byte by eight once, which
 * is element index route * 2, and adds it to each table base, so both bases stay
 * their own symbol.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32 D_80181B98[];

/* Work-record table of the shared main-executable data segment: 0x140-byte
 * records based at 0x80145E90, counted by the byte at 0x80146254. This overlay
 * only hands those records to the shared position gate below without reading a
 * field of its own, so the record stays a byte view here.
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

/* @source 0x801CFBDC
 * @behavior Tests the projected position of one front-end record against the
 *           work records of the shared main-executable data segment. The
 *           argument is the record slot; its 0x74-byte row offset selects the
 *           row word x at 0x80143FFC, the row word y at 0x80144000 and the row
 *           halfword at 0x80144006. Before the scan it reads the active work
 *           area through the scratchpad pointer cell 0x1F800044 and takes that
 *           record's route byte at 0x08, which selects an eight-byte movement
 *           offset record: the x offset from 0x80181B94 and the y offset from
 *           0x80181B98, each doubled before it is added to the matching row
 *           word. Starting at work record 1 and while the counter stays below
 *           the record count byte D_80146254, it passes the projected pair, the
 *           row halfword as a signed reference, the range 1 and the 0x140-byte
 *           work record to the shared gate func_801BDD58 and returns 1 as soon
 *           as that gate reports a match. Otherwise, and whenever the count byte
 *           is not above 1, it returns 0. Work record 0 is never tested. The
 *           original computes the row offset once before the loop and keeps it
 *           in a saved register while the loop body rebuilds the halfword's
 *           symbol base from it; the projected pair stays in registers across
 *           the whole scan, and the work-record address is computed first in the
 *           loop body but stored into the argument slot after the range
 *           constant. Neither this function's address nor any pointer to it
 *           appears in this payload, so it is an overlay entry reached from
 *           outside the payload and its prologue and return paths alone place
 *           the boundary. Its 0x11C-byte original range is byte-identical to the
 *           exact lift of this body already claimed by the plp025 overlay at
 *           0x801CF808, which is why the statement order, operand forms and
 *           declaration style here are those of that body.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 testProjectedFrontEndRecordPositionAgainstWorkRecordsPlcharPlp01200_801CFBDC(
    u8 slot) {
  void* work;
  s32 i;
  s32 x;
  s32 y;
  u8 route;
  u32 row;
  u32 projectionRow;

  projectionRow = (u32)slot * 0x74u;
  route = SPAD_PTR_SLOT(u8, 0x44u)[8];
  x = *(s32*)(D_80143FFC + projectionRow) + (D_80181B94[route * 2] << 1);
  y = *(s32*)(D_80144000 + projectionRow) + (D_80181B98[route * 2] << 1);
  for (i = 1; i < D_80146254; i++) {
    /* Keep the reference row's lifetime separate from the projected pair. */
    row = (u32)slot * 0x74u;
    work = &D_80145E90[i * 0x140];
    if (func_801BDD58(x, y, *(s16*)(D_80144006 + row), 1, work) != 0) {
      return 1;
    }
  }
  return 0;
}
