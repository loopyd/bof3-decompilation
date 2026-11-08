#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads this cell ahead of frame creation for the countdown read, then
 * re-reads it after the countdown store, after the cue call, after the slot
 * search, after the row seed and after the row seed's own step increment,
 * because a store through the pointer may alias the cell itself. The source
 * keeps the named cell and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the active work area's
 * route byte at offset 0x08. The original scales that byte by eight once, which
 * is element index route * 2, and doubles each offset before it adds it, so both
 * bases stay their own symbol. The exact lifts of this target declare the same
 * two tables the same way.
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

/* Cue dispatcher of the shared EXE image; the exact steps at 0x801D04F0 and
 * 0x801CFF24 of this target declare the same prototype. This body passes the
 * work record's unsigned halfword 0x2C plus 0x100 and the literal 0x10B.
 */
extern void func_8015DF18(u32 cue);

/* Readiness poll helper of the main executable; this target's steps declare the
 * same prototype. This body calls it last on every path and discards the answer.
 */
extern u8 func_8014DAEC(void);

/* Slot search of the shared EXE image, outside this payload: it takes no
 * arguments, projects the same active work area with the undoubled offsets of
 * 0x80181B94/0x80181B98 and walks the twenty 0x74-byte front-end rows at
 * 0x80143FC8 whose byte 0 reads nonzero and whose byte 5 reads 0x17. It answers
 * the signed slot of the first row it accepts, or -1 when none is. This body
 * seeds the row this search reports and records the slot, so the answer is read
 * as a signed byte and tested against -1.
 */
s8 func_801B5AE4(void);

/* Range slot search of the same shared image: it walks the twenty 0x74-byte rows
 * whose byte 5 reads 0x30 and answers the signed slot of the first row that holds
 * the supplied point at the supplied range, or -1 when no row does. This body
 * passes the projected point at range 0 and raises the animation bit of the
 * answered row.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Map cell claim of this payload, still an `asm` row at 0x801D0148: it
 * sign-extends its two signed halves, probes the map cell code through the shared
 * func_80166CB0 and claims a front-end row for the accepted codes. It answers one
 * when the claim succeeded and zero when it did not, and this body tests only the
 * low byte of that answer, so the declared return is that byte. Both cell offsets
 * are signed sixteen-bit values, so the original converts each retry index to
 * that width before the call.
 */
u8 claimFrontEndRowForMapCodesF0F1F4OrRandomRowForF6F7ThenStageNameOrSetWorkByteBPlcharPlp02600_801D0148(
    s16 col, s16 row);

/* @source 0x801CFCD8
 * @behavior Work byte 3 == 1 step of the work-byte-3 dispatcher at 0x801CFB94.
 *           This function is entry 1 of the five-word run the dispatcher reaches
 *           from its base word 0x801D0A94 with work record byte 3 read through
 *           the scratchpad pointer cell 0x1F800044, the word 0x801CFCD8 at
 *           0x801D0A98, entry 18 of the thirty-word code-pointer run based at
 *           0x801D0A54. No jal in this payload names this function, so that run
 *           is its only in-image reference. It decrements the work record's
 *           countdown byte 0x0A, armed with 0xB by the preceding step at
 *           0x801CFBD8, and does nothing else while that byte stays nonzero.
 *           When the countdown reaches zero it queues the cue computed from the
 *           work record's unsigned halfword 0x2C plus 0x100 through the shared
 *           cue dispatcher func_8015DF18 and asks the no-argument slot search
 *           func_801B5AE4 for a front-end row. When that search answers a slot
 *           rather than -1 the step writes 1 into field 0x0A and the work route
 *           byte 0x08 into field 0x08 of that row, whose stride is 0x74, copies
 *           the answered slot into work byte 0x06 through the pointer cell, and
 *           advances work byte 3. Otherwise the step projects the work position
 *           by adding the doubled movement offsets of 0x80181B94 and 0x80181B98
 *           selected by the route byte 0x08 to the work words 0x34 and 0x38,
 *           hands that pair to the range-0 slot search func_801BDE14 and, unless
 *           that search answers -1, raises bit 0 of the animation byte 0x74 of
 *           the found work record through the column base 0x801468FC and queues
 *           the literal cue 0x10B. It then claims a front-end row at the
 *           projected point through this payload's map cell claim handler
 *           claimFrontEndRowForMapCodesF0F1F4OrRandomRowForF6F7ThenStageNameOrSetWorkByteB,
 *           passing the high half of each projected word: when
 *           that claim fails and the first word has a nonzero low half it retries
 *           one unit further on that axis, and when the retry also fails and the
 *           second word has a nonzero low half it retries one unit further on the
 *           second axis. Every path then increments the step counter byte 0x03,
 *           so the accepted-row arm advances it twice, and the body closes by
 *           running the shared readiness helper func_8014DAEC. It takes no
 *           arguments and returns nothing. The 0x30-byte frame keeps $ra at 0x28
 *           and the six saved registers across the calls, the range constant 0 is
 *           materialised in the delay slot of the branch into the projected
 *           path, and the cell load that opens the body issues before the frame
 *           is created. A little-endian scan of the extracted payload corpus for
 *           this 0x24C-byte span finds no byte-identical twin at all, so the body
 *           is seeded from its own bytes; the closest row-field shapes are the
 *           exact twins of the cue-then-seed step at 0x801D04F0 and of the
 *           projected-claim step at 0x801CEA94 of plp012, and both differ from
 *           this body in their call set and in their countdown tail. The roles of
 *           work bytes 0x03, 0x06, 0x08 and 0x0A, of the two front-end row
 *           fields, of the cues and of the two slot searches are not established,
 *           so the name describes the observed step and those stay described by
 *           address or by the effect the step has on them. Original bytes: the
 *           body reaches the byte 0x0A decrement through one cell load and reads
 *           the cell again for the second countdown test, the success arm keeps
 *           the row scale product in the register the multiply chain ends in and
 *           re-materialises each row field base through %hi/%lo, and the
 *           projected arm keeps one base for the three work-record reads. Two
 *           byte offsets are named separately below because the original computes
 *           the 0x74 row product and the 0x98 animation product into their own
 *           temporaries; naming them separately alone restored the success arm's
 *           instruction sequence. The measured residual is register naming only:
 *           the emitted instruction sequence, order and size are the original's,
 *           but this compiler build allocates the searched slot's unsigned byte
 *           and its sign-extended copy to $s1/$s0 where the original uses
 *           $s0/$s1, and then allocates the projected first word and its high
 *           half to $s1/$s2 where the original uses $s2/$s1. Fifteen of the 147
 *           instructions differ, all of them across those two spans, and every
 *           one of them is a register operand of an otherwise identical opcode.
 * @status partial
 * @match 89.80
 * @residual 132/147 instructions and 588 of 588 bytes: the searched slot and its
 *           sign-extended copy, and the projected first word and its high half,
 *           are coloured into each other's registers ($s0/$s1 and $s1/$s2), with
 *           opcode order and size unchanged.
 */
void queueCue2CPlus100ThenSeedFrontEndRowFoundBySlotSearchElseClaimProjectedCellAndAdvanceWorkByte3PlcharPlp02600_801CFCD8(
    void) {
  u8* work;
  u8 route;
  u8 claimed;
  s8 slot;
  s8 found;
  s32 x;
  s32 y;
  s32 off;
  s32 aoff;
  s16 cx;
  s16 cy;

  if (D_1F800044[0x0A] != 0) {
    D_1F800044[0x0A] -= 1;
    if (D_1F800044[0x0A] == 0) {
      func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
      slot = func_801B5AE4();
      if (slot != -1) {
        func_8015DF18(0x10B);
        off = slot * 0x74;
        route = D_1F800044[0x08];
        D_80143FD2[off] = 1;
        D_80143FD0[off] = route;
        D_1F800044[0x06] = slot;
        D_1F800044[0x03] += 1;
      } else {
        work = D_1F800044;
        route = work[0x08];
        x = *(s32*)(work + 0x34) + D_80181B94[route * 2] * 2;
        y = *(s32*)(work + 0x38) + D_80181B98[route * 2] * 2;
        found = func_801BDE14(x, y, 0);
        if (found != -1) {
          aoff = found * 0x98;
          D_801468FC[aoff] = D_801468FC[aoff] | 1;
          func_8015DF18(0x10B);
        }
        cx = x >> 16;
        cy = y >> 16;
        claimed = claimFrontEndRowForMapCodesF0F1F4OrRandomRowForF6F7ThenStageNameOrSetWorkByteBPlcharPlp02600_801D0148(
            cx, cy);
        if (claimed == 0) {
          if ((x & 0xFFFF) != 0) {
            claimed = claimFrontEndRowForMapCodesF0F1F4OrRandomRowForF6F7ThenStageNameOrSetWorkByteBPlcharPlp02600_801D0148(
                cx + 1, cy);
          }
          if (claimed == 0 && (y & 0xFFFF) != 0) {
            claimFrontEndRowForMapCodesF0F1F4OrRandomRowForF6F7ThenStageNameOrSetWorkByteBPlcharPlp02600_801D0148(
                cx, cy + 1);
          }
        }
      }
      D_1F800044[0x03] += 1;
    }
  }
  func_8014DAEC();
}
