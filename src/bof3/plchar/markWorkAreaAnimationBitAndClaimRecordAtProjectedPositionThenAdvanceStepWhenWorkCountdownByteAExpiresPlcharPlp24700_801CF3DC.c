#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after the countdown store of the body and once
 * more for the closing step counter store, because a store through the pointer
 * may alias the cell itself, so the source keeps the named-symbol form and
 * evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the active work area's
 * route byte at offset 0x08. The original scales that byte by eight once, which
 * is element index route * 2, and doubles each offset before it adds it, so both
 * bases stay their own symbol. This target's generated undefined_syms_auto.txt
 * binds both addresses.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32 D_80181B98[];

/* Animation byte column of the shared work-record table, 34 records of 0x98
 * bytes based at 0x80146888: the byte at 0x801468FC is field 0x74 of a record,
 * the animation halfword low byte whose bit 0 this body raises for the record
 * that holds the projected position. The original multiplies the record slot by
 * 0x98 against this column base instead of adding a row offset to the table
 * base, so the column keeps an address-derived symbol of its own.
 * @source 0x801468FC @kind unknown
 */
extern u8 D_801468FC[];

/* Map cell claim of this same payload, still `asm` in this target: it
 * sign-extends both arguments through `sll` plus `sra` in its own prologue, asks
 * the shared map probe for the one-byte code of that cell, claims a front-end
 * record for the codes it handles and answers one when a record was claimed and
 * zero otherwise. The original passes the projected point's two high halves as
 * signed halves, and the retry paths let the compiler narrow the stepped
 * coordinate to the same signed half before the call.
 */
u8 claimRecordForMapCodeF2WithRandomSizeElseStageItemNameCueForMapCodeF8ThenSetWorkByteBAndPublishPositionPlcharPlp24700_801CF664(
    s16 x, s16 y);

/* Slot search of the shared EXE image, outside this payload: it answers the
 * signed slot of the first eligible 0x98-byte work record that holds the
 * supplied point at the supplied range, or -1 when no record does. This
 * target's generated undefined_funcs_auto.txt binds the address, and the
 * original passes the range constant 0 as the third argument.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Shared readiness helper of the shared EXE image, outside this payload. This
 * body calls it on both paths and discards the returned byte. */
u8 func_8014DAEC(void);

/* @source 0x801CF3DC
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as word
 *           0x801CF3DC at 0x801CFB2C, entry 12 of the twenty-word code pointer
 *           run based at 0x801CFAFC. This overlay's byte-2 work dispatcher
 *           indexes that run from its own base 0x801CFB1C, where entry 12 sits
 *           at index 4, so the loader that steps the run on work byte 2 equal
 *           to 4 installs this body. It decrements the countdown byte 0x0A of
 *           the work object and,
 *           while that byte stays nonzero, only runs the shared readiness helper
 *           func_8014DAEC. When the byte reaches zero the step projects the work
 *           position: it reads route byte 0x08, scales it by eight and adds the
 *           doubled movement offsets of 0x80181B94 and 0x80181B98 to work words
 *           0x34 and 0x38. It hands that pair with range 0 to the shared slot
 *           search func_801BDE14 and, unless the search answers -1, raises bit 0
 *           of the animation byte at 0x801468FC plus the found slot scaled by
 *           the 0x98-byte work record stride. It then claims a front-end record
 *           at the projected point through this payload's own claim handler at
 *           0x801CF664, passing the high half of each projected word as signed
 *           halves: when that claim answers zero and the first word has a
 *           nonzero low half it retries one unit further on that axis, and when
 *           the retry also answers zero and the second word has a nonzero low
 *           half it retries one unit further on the second axis, discarding the
 *           last answer. The projected second word keeps a second live copy
 *           because the two cell halves claim the register that holds it, so the
 *           last retry test reads that copy and the source binds it to a variable
 *           of its own. It then increments step byte 0x02 through the scratchpad
 *           cell itself, and the body closes by running the shared readiness
 *           helper func_8014DAEC on every path. It takes no arguments and
 *           returns nothing. The 0x30-byte frame keeps $ra at 0x28 and the six
 *           saved registers across the calls, the range constant is materialised
 *           in the delay slot of the branch that leaves the expiry path, and the
 *           increment of byte 0x02 issues before the closing shared call.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape note from the original bytes: the 0x19C-byte body differs from the
 * exact plp012/00, plp015/00, plp016/00 and plp025/00 lifts of the span at
 * 0x801CEA94 in exactly three words, the three call words that name the claim
 * handler. Each of those shifts by the same delta 0x948 because this payload
 * places that handler at 0x801CF664 instead of 0x801CED1C. The exact plp257/00
 * lift of 0x801CF7BC differs from this body in those same three words alone,
 * each shifted by 0x3E0, and the plp27a/00 lift of 0x801CF1C0 in those same
 * three words at 0x21C. Equal deltas across every differing word make one body,
 * so the statement order, operand forms, offset spelling and declaration style
 * of this source follow the exact plp012/00 lift of that span, whose call words
 * this source writes with this payload's own handler. The body was therefore
 * transposed from that lift rather than cloned: three relocation words differ,
 * so no copy of this span is byte-identical to this payload.
 */
void markWorkAreaAnimationBitAndClaimRecordAtProjectedPositionThenAdvanceStepWhenWorkCountdownByteAExpiresPlcharPlp24700_801CF3DC(
    void) {
  u8* work;
  u8 route;
  s32 x;
  s32 y;
  s8 slot;
  u8 claimed;
  s16 cx;
  s16 cy;
  s32 off;
  s32 y2;

  D_1F800044[0x0A] -= 1;
  work = D_1F800044;
  if (work[0x0A] == 0) {
    route = work[0x08];
    x = *(s32*)(work + 0x34) + D_80181B94[route * 2] * 2;
    y = *(s32*)(work + 0x38) + D_80181B98[route * 2] * 2;
    y2 = y;
    slot = func_801BDE14(x, y, 0);
    if (slot != -1) {
      off = slot * 0x98;
      D_801468FC[off] |= 1;
    }
    cx = x >> 16;
    cy = y >> 16;
    claimed = claimRecordForMapCodeF2WithRandomSizeElseStageItemNameCueForMapCodeF8ThenSetWorkByteBAndPublishPositionPlcharPlp24700_801CF664(
        cx, cy);
    if ((claimed & 0xFF) == 0) {
      if ((x & 0xFFFF) != 0) {
        claimed = claimRecordForMapCodeF2WithRandomSizeElseStageItemNameCueForMapCodeF8ThenSetWorkByteBAndPublishPositionPlcharPlp24700_801CF664(
            cx + 1, cy);
      }
      if ((claimed & 0xFF) == 0 && (y2 & 0xFFFF) != 0) {
        claimRecordForMapCodeF2WithRandomSizeElseStageItemNameCueForMapCodeF8ThenSetWorkByteBAndPublishPositionPlcharPlp24700_801CF664(
            cx, cy + 1);
      }
    }
    D_1F800044[0x02] += 1;
  }
  func_8014DAEC();
}
