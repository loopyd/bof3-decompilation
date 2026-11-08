#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads this cell ahead of frame creation for the countdown read and
 * re-reads it after the countdown store, after the expiry cue and at each later
 * access group, because a store through the pointer may alias the cell itself.
 * The source keeps the named cell and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

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
 * rows based at 0x80143FC8. This body seeds two row fields through their own
 * absolute symbols, because the original re-materialises each field base and
 * adds the scaled row offset again instead of keeping one row base.
 * @source 0x80143FD0 @kind unknown
 */
extern u8 D_80143FD0[];
/* Row field of the same 0x74-byte front-end row as D_80143FD0, two bytes on.
 * @source 0x80143FD2 @kind unknown
 */
extern u8 D_80143FD2[];

/* Cue dispatcher of the shared EXE image, outside this payload. This body queues
 * the work record's unsigned halfword 0x2C plus 0x100 on expiry and the literal
 * cue 0x10B on both the seeded-row and the marked-slot path. */
extern void func_8015DF18(u32 cue);

/* No-argument slot search of the shared EXE image, outside this payload: it
 * walks the front-end rows at 0x80143FC8 that carry the free handler selector
 * and answers the signed slot of the first row it accepts, or -1 when none is.
 * This body seeds the row the answer indexes, records that answer in work byte
 * 0x06 and tests it against -1, so the answer is read both as its raw byte and
 * as a signed byte. */
s8 func_801B5AE4(void);

/* Range slot search of the same shared image: it walks the 0x98-byte work-record
 * table whose byte 5 reads the work handler selector and answers the signed slot
 * of the first record that holds the supplied point at the supplied range, or -1
 * when no record does. This body passes the projected point at range 0 and
 * raises the animation bit of the answered record. */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Map cell claim of this same payload, the exact lift of its own row at
 * 0x801CF420: its prologue sign-extends both arguments through `sll 16` plus
 * `sra 16`, probes the map cell code through the shared func_80166CB0 and claims
 * a front-end record through 0x801CF62C for the codes it accepts. It answers one
 * for those codes and zero otherwise, and this body tests only the low byte of
 * that answer. Both cell offsets are signed sixteen-bit values, so the original
 * converts each retry index to that width before the call.
 */
u8 claimFrontEndRecordForMapCodesF0F1F4F6F7WithRandomStateElseStageItemNameStream29CueThenSetWorkByteBPlcharPlp26700_801CF420(
    s16 col, s16 row);

/* Shared readiness helper of the shared EXE image, outside this payload. This
 * body calls it last on every path and discards the returned byte. */
u8 func_8014DAEC(void);

/* @source 0x801CEFB0
 * @behavior Work byte 3 == 1 step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, entry 1 of the five-word
 *           code-pointer run based at 0x801D09B0 that the byte-3 dispatcher of
 *           this payload at 0x801CEE6C indexes with work byte 3 scaled by four,
 *           and so entry 11 of the thirty-word code-pointer run based at
 *           0x801D0988 that this target places as the textbin row T_801D0988.
 *           The body address appears once in the payload bytes, as the run word
 *           at 0x801D09B4, and no jal in this payload names it. While the work
 *           countdown byte 0x0A reads nonzero the step only decrements it and
 *           writes nothing else. When the countdown reaches zero it queues the
 *           cue computed from the work record's unsigned halfword 0x2C plus 0x100
 *           through the shared cue dispatcher func_8015DF18 and asks the
 *           no-argument slot search func_801B5AE4 for a front-end row. When that
 *           answer is not -1 the step queues the literal cue 0x10B, writes 1
 *           into field 0x0A and the work route byte 0x08 into field 0x08 of the
 *           0x74-byte row the answer indexes, copies the answer byte itself into
 *           work byte 0x06 and advances work byte 3, so that path leaves the
 *           expiry with the step counter raised twice. When the answer is -1 the
 *           step instead projects the work position by adding the doubled
 *           movement offsets of 0x80181B94 and 0x80181B98 selected by the route
 *           byte 0x08 to the work words 0x34 and 0x38, hands that pair to the
 *           range-0 slot search func_801BDE14 and, unless that search answers -1
 *           as well, raises bit 0 of the animation byte 0x74 of the found work
 *           record through the column base 0x801468FC and the slot stride 0x98
 *           and queues the literal cue 0x10B. It then claims a front-end record
 *           at the projected point through this payload's claim at 0x801CF420,
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
 *           test. A masked little-endian scan of the extracted payload corpus for
 *           this 0x24C-byte span finds six siblings, at the plp467, plp016,
 *           plp026, plp056, plp567 and plp678 spans, and each differs from this
 *           body in four words only: the three jal relocations of its own claim
 *           and the one internal jump word, every one of which shifts by exactly
 *           the delta between the two spans, while all remaining 143 words stay
 *           equal, so those copies are one shared body rather than a similar one.
 *           No copy is byte-identical, so this span is seeded from the exact lift
 *           of the same body at 0x801CF8B4 of plp016, which differs only in those
 *           relocation words. The roles of work byte 0x06, of the two seeded row
 *           bytes and of the answers beyond their effect are not established, so
 *           this block describes that effect only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCue2CPlus100ThenSeedFrontEndRowFoundBySlotSearchElseClaimProjectedCellAndAdvanceWorkByte3PlcharPlp26700_801CEFB0(
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
        claimed = claimFrontEndRecordForMapCodesF0F1F4F6F7WithRandomStateElseStageItemNameStream29CueThenSetWorkByteBPlcharPlp26700_801CF420(
            cx, cy);
        if ((claimed & 0xFF) == 0) {
          if ((x & 0xFFFF) != 0) {
            claimed = claimFrontEndRecordForMapCodesF0F1F4F6F7WithRandomStateElseStageItemNameStream29CueThenSetWorkByteBPlcharPlp26700_801CF420(
                cx + 1, cy);
          }
          if ((claimed & 0xFF) == 0 && (y & 0xFFFF) != 0) {
            claimFrontEndRecordForMapCodesF0F1F4F6F7WithRandomStateElseStageItemNameStream29CueThenSetWorkByteBPlcharPlp26700_801CF420(
                cx, cy + 1);
          }
        }
      }
      D_1F800044[0x03] += 1;
    }
  }
  func_8014DAEC();
}
