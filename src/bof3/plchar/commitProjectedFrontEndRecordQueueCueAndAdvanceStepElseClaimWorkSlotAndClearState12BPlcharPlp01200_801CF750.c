#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell before every access group of this body, because a
 * store through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at each access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The body writes
 * the shared work mode byte 0x11C and the state byte 0x12B of this record and
 * reads its bit 0 of byte 0x12C.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* World-flag halfword of the shared main executable. The commit path sets bit
 * 0x1000 of it. No lift of this executable names that flag yet, so the halfword
 * and the bit stay address-anchored.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses one field through its own
 * absolute symbol, because the original re-materialises that base and adds the
 * scaled row offset for the store.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 0x0B, seeded with 1 by the commit path.
 * @source 0x80143FD3 @kind unknown
 */
extern u8 D_80143FD3[];

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the active work area's
 * route byte at offset 0x08. The original scales that byte by eight once, which
 * is element index route * 2, and doubles each offset before it adds it, so both
 * bases stay their own symbol.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32 D_80181B98[];

/* Animation byte column of the shared work-record table, 34 records of 0x98
 * bytes based at 0x80146888: the byte at 0x801468FC is field 0x74 of a record,
 * the animation halfword low byte whose bit 0 the claim path raises for the
 * work record that holds the projected position.
 * @source 0x801468FC @kind unknown
 */
extern u8 D_801468FC[];

/* Record lookup of this same payload: it answers the index of the first
 * front-end record that holds the projected work-area position, drifting its
 * probe along the route, or -1 when no record does. The original masks the
 * signed answer into an unsigned byte once at entry.
 */
s8 findFrontEndRecordIndexAtProjectedWorkPositionPlcharPlp01200_801CF9A8(void);

/* Position test of this same payload: it answers nonzero when the front-end
 * record with the supplied index holds the position of a work record. */
u8 testFrontEndRecordPositionAgainstWorkRecordsPlcharPlp01200_801CFB0C(u8 slot);
/* Position test of this same payload: it answers nonzero when the supplied
 * front-end record holds its own position projected along its route. */
u8 testProjectedFrontEndRecordPositionAgainstWorkRecordsPlcharPlp01200_801CFBDC(
    u8 slot);

/* Slot search of the shared EXE image, outside this payload: it answers the
 * signed slot of the first eligible 0x98-byte work record that holds the
 * supplied point at the supplied range, or -1 when no record does.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Cue queue of the shared main executable, outside this payload: it takes the
 * cue identifier the original materialises from the work halfword 0x2C plus
 * 0x100 and reads no result from $v0. */
void func_8015DF18(u32 cue);

/* Sequence selection of the shared main executable, outside this payload: it
 * takes the one-byte sequence identifier the original materialises from the
 * work route byte 0x08 plus 8 and reads no result from $v0. */
u32 func_8014D8D4(u8 arg0);

/* Shared no-argument update helper of the main executable, outside this
 * payload. The original sets nothing up in the argument registers and reads no
 * result from $v0. */
void func_801C24D0(void);

/* Work advance step of the shared main executable, outside this payload: it
 * moves the work record by its current delta. The original sets nothing up in
 * the argument registers and reads no result from $v0. */
void func_801B0DFC(void);

/* @source 0x801CF750
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached only through the
 *           eight-word pointer run based at 0x801CFDB8 that func_801CF70C
 *           indexes with the work step counter byte 0x02: this body's own
 *           address is word 0 of that run, which is also word 14 of the
 *           sixteen-word run starting at 0x801CFD80, and no jal in this payload
 *           names it. The body first asks the record lookup at 0x801CF9A8 for
 *           the front-end record that holds the projected work position and
 *           masks its answer into an unsigned index byte. When that index is
 *           0xFF, meaning no record was found, it claims a work slot instead:
 *           it projects the work words 0x34 and 0x38 by the movement offsets of
 *           0x80181B94 and 0x80181B98 selected by the route byte 0x08 and
 *           doubled, and unless bit 0 of byte 0x12C of the front-end record
 *           published at 0x80146250 is set, it hands that pair to the slot
 *           search func_801BDE14 with range 1 and, when that answers a slot
 *           other than the 0xFF it was given, raises bit 0 of the animation byte
 *           0x74 of the found work record through the column base 0x801468FC and
 *           the slot stride 0x98. When the lookup did find a record, the body
 *           commits only if neither position test of this payload, at
 *           0x801CFB0C and 0x801CFBDC, reports that record as already holding a
 *           work position; the commit seeds row byte 0x0B of the front-end row
 *           through the base 0x80143FD3 and the row stride 0x74, queues the cue
 *           built from the work halfword 0x2C plus 0x100 through func_8015DF18,
 *           selects the sequence built from the work route byte 0x08 plus 8
 *           through func_8014D8D4, sets the shared work mode byte 0x11C of the
 *           record published at 0x80146250 to 2 and raises bit 0x1000 of the
 *           world-flag halfword 0x80146258, runs the shared update func_801C24D0,
 *           decrements work byte 0x09 and then runs the work advance step
 *           func_801B0DFC, writes 1 into state byte 0x12B of that record and
 *           finally increments the work step counter byte 0x02. Every other path
 *           - both position tests passing, or the claim path finishing - clears
 *           state byte 0x12B instead, so the commit is the only path that leaves
 *           it set. The body takes no arguments, returns nothing and answers no
 *           value. Its 0x18-byte frame keeps $ra at 0x14 and $s0 at 0x10, the
 *           index byte stays in $s0 across the calls, the constant 1 stays in
 *           that register for both stores of the commit path, the closing
 *           step counter increment issues before the jump to the shared
 *           epilogue, and the animation store keeps the record slot in a
 *           signed local of its own so the original's `lui $at` column
 *           addressing form survives.
 * @status exact
 * @match 100.00
 * @residual none
 */
void commitProjectedFrontEndRecordQueueCueAndAdvanceStepElseClaimWorkSlotAndClearState12BPlcharPlp01200_801CF750(
    void) {
  u8 slot;
  u8 route;
  s32 off;
  s32 x;
  s32 y;

  slot = findFrontEndRecordIndexAtProjectedWorkPositionPlcharPlp01200_801CF9A8();
  if (slot != 0xFF) {
    if (testFrontEndRecordPositionAgainstWorkRecordsPlcharPlp01200_801CFB0C(slot) ==
            0 &&
        testProjectedFrontEndRecordPositionAgainstWorkRecordsPlcharPlp01200_801CFBDC(
            slot) == 0) {
      D_80143FD3[(u32)slot * 0x74u] = 1;
      func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
      func_8014D8D4(D_1F800044[0x08] + 8);
      D_80146250[0x11C] = 2;
      D_80146258 |= 0x1000;
      func_801C24D0();
      D_1F800044[0x09] -= 1;
      func_801B0DFC();
      D_80146250[0x12B] = 1;
      D_1F800044[0x02] += 1;
      return;
    }
  } else {
    route = D_1F800044[0x08];
    x = *(s32*)(D_1F800044 + 0x34) + D_80181B94[route * 2] * 2;
    y = *(s32*)(D_1F800044 + 0x38) + D_80181B98[route * 2] * 2;
    if ((D_80146250[0x12C] & 1) == 0) {
      slot = func_801BDE14(x, y, 1);
      if (slot != 0xFF) {
        off = slot * 0x98;
        D_801468FC[off] |= 1;
      }
    }
  }
  D_80146250[0x12B] = 0;
}
