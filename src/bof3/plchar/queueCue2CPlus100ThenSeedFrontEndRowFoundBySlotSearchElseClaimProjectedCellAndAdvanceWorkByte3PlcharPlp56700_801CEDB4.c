#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it before the
 * countdown read that opens the body, again for the second countdown test, again
 * for the route byte and the two front-end row fields, and again before each of
 * the three work-byte stores, so the source keeps the named symbol form and
 * evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the active work area route
 * byte at offset 0x08. The original scales that byte by eight once, which is
 * element index route * 2, and doubles each offset before it adds it, so both
 * bases stay their own symbol.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32 D_80181B98[];

/* Animation byte column of the shared work-record table, 0x98-byte records based
 * at 0x80146888: the byte at 0x801468FC is field 0x74 of a record. This body
 * raises bit 0 of that byte for the record the range-0 slot search reports, and
 * the original multiplies the record slot by 0x98 against this column base
 * instead of adding a row offset to the table base, so the column keeps an
 * address-derived symbol of its own.
 * @source 0x801468FC @kind unknown
 */
extern u8 D_801468FC[];

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses two row fields through their own
 * absolute symbols, because the original re-materialises each field base and
 * adds the scaled row offset again instead of keeping one row base.
 * @source 0x80143FD0 @kind unknown
 */
extern u8 D_80143FD0[];
/* Row field of the same 0x74-byte front-end row as D_80143FD0, two bytes on.
 * @source 0x80143FD2 @kind unknown
 */
extern u8 D_80143FD2[];

/* Cue dispatcher of the shared EXE image; this target's steps declare the same
 * prototype. This body passes the work record's unsigned halfword 0x2C plus
 * 0x100 and the literal 0x10B.
 */
extern void func_8015DF18(u32 cue);

/* Readiness poll helper of the main executable; this target's steps declare the
 * same prototype. This body calls it last on every path and discards the answer.
 */
extern u8 func_8014DAEC(void);

/* No-argument slot search of the shared EXE image, outside this payload: it walks
 * the 0x74-byte front-end rows at 0x80143FC8 and answers the signed slot of the
 * first row it accepts, or -1 when none is. This body seeds the row that search
 * reports and tests the answer against -1, so the answer is read both as its raw
 * byte and as a signed byte.
 */
s8 func_801B5AE4(void);

/* Range slot search of the same shared image: it walks the rows whose byte 5
 * reads 0x30 and answers the signed slot of the first row that holds the supplied
 * point at the supplied range, or -1 when no row does. This body passes the
 * projected point at range 0 and raises the animation bit of the answered row.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Map cell claim of this payload, still an `asm` row at 0x801CF224: it
 * sign-extends its two signed halves, probes the map cell code through the shared
 * func_80166CB0 and claims a front-end record for the accepted codes. It answers
 * one when the claim succeeded and zero when it did not, and this body tests only
 * the low byte of that answer. Both cell offsets are signed sixteen-bit values,
 * so the original converts each retry index to that width before the call. Its
 * own Splat row sits before this one in the payload, so the symbol is defined
 * inside this payload and needs no foreign binding.
 */
u8 func_801CF224(s16 col, s16 row);

/* @source 0x801CEDB4
 * @behavior Work byte 3 == 1 step of the second byte-3 dispatch group of this
 *           overlay: entry 1 of the handler run that the byte-3 dispatcher
 *           D_801D07B8 indexes, the word 0x801CEDB4 at 0x801D07BC of the
 *           in-image pointer run based at 0x801D078C. The preceding step at
 *           0x801CECB4 writes 1 into work byte 3 to advance to it. It decrements
 *           the work countdown byte 0x0A, armed with 0xB by that preceding step,
 *           and does nothing else while the byte stays nonzero. When the
 *           countdown reaches zero it queues the cue built from the work
 *           record's unsigned halfword 0x2C plus 0x100 through the shared cue
 *           dispatcher func_8015DF18 and asks the no-argument slot search
 *           func_801B5AE4 for a front-end row. When that search answers a slot
 *           rather than -1 the step writes 1 into row field 0x0A and the work
 *           route byte 0x08 into row field 0x08 of that row, whose stride is
 *           0x74, queues cue 0x10B, copies the answered byte itself into work
 *           byte 0x06 and advances work byte 3 here, so that path leaves the
 *           expiry with the step counter raised twice. When the answer is -1 the
 *           step instead projects the work position by adding the doubled
 *           movement offsets of 0x80181B94 and 0x80181B98 selected by the route
 *           byte 0x08 to the work words 0x34 and 0x38, hands that pair to the
 *           range-0 slot search func_801BDE14 and, unless that search answers -1
 *           as well, raises bit 0 of the animation byte 0x74 of the found work
 *           record through the column base 0x801468FC and the slot stride 0x98
 *           and queues the literal cue 0x10B. It then claims a front-end record
 *           at the projected point through this payload's claim at 0x801CF224,
 *           passing the high half of each projected word: when that claim answers
 *           zero and the first word has a nonzero low half it retries one unit
 *           further on that axis, and when the retry also answers zero and the
 *           second word has a nonzero low half it retries one unit further on the
 *           second axis, discarding that last answer. Every path then increments
 *           work byte 3 once more and the body closes by running the shared
 *           readiness helper func_8014DAEC. It takes no arguments and returns
 *           nothing. The 0x30-byte frame keeps $ra at 0x28 and the six saved
 *           registers $s5 to $s0 at 0x24 to 0x10 across the calls, the pointer
 *           cell is read before the frame is created, and the range constant 0 is
 *           materialised in the delay slot of the branch that leaves the answer
 *           test. The roles of work byte 0x06, of the two seeded row bytes and of
 *           the answers beyond their effect are not established, so this block
 *           describes that effect only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCue2CPlus100ThenSeedFrontEndRowFoundBySlotSearchElseClaimProjectedCellAndAdvanceWorkByte3PlcharPlp56700_801CEDB4(
    void) {
  u8 answer;
  s8 slot;
  s32 signed_answer;
  u8 route;
  u8 b8;
  s32 off;
  s32 x;
  s32 y;
  s16 cx;
  s16 cy;
  u8 claimed;

  if (D_1F800044[0x0A] != 0) {
    D_1F800044[0x0A] = D_1F800044[0x0A] - 1;
    if (D_1F800044[0x0A] == 0) {
      func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
      answer = func_801B5AE4();
      signed_answer = (s8)answer;
      if (signed_answer != -1) {
        func_8015DF18(0x10B);
        b8 = D_1F800044[0x08];
        D_80143FD2[(u32)signed_answer * 0x74u] = 1;
        D_80143FD0[(u32)signed_answer * 0x74u] = b8;
        D_1F800044[0x06] = answer;
        D_1F800044[0x03] += 1;
      } else {
        route = D_1F800044[0x08];
        x = *(s32*)(D_1F800044 + 0x34) + D_80181B94[route * 2] * 2;
        y = *(s32*)(D_1F800044 + 0x38) + D_80181B98[route * 2] * 2;
        answer = func_801BDE14(x, y, 0);
        slot = (s8)answer;
        if (slot != signed_answer) {
          off = slot * 0x98;
          D_801468FC[off] |= 1;
          func_8015DF18(0x10B);
        }
        cx = x >> 16;
        cy = y >> 16;
        claimed = func_801CF224(cx, cy);
        if ((claimed & 0xFF) == 0) {
          if ((x & 0xFFFF) != 0) {
            claimed = func_801CF224(cx + 1, cy);
          }
          if ((claimed & 0xFF) == 0 && (y & 0xFFFF) != 0) {
            func_801CF224(cx, cy + 1);
          }
        }
      }
      D_1F800044[0x03] += 1;
    }
  }
  func_8014DAEC();
}
