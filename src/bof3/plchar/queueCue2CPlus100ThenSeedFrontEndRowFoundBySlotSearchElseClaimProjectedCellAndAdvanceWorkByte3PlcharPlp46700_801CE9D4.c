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
 * second-coordinate offset at 0x80181B98, selected by the active work area
 * route byte at offset 0x08. The original scales that byte by eight once, which
 * is element index route times two, and doubles each offset before it adds it,
 * so both bases stay their own symbol. This target generated
 * undefined_syms_auto.txt binds both addresses.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table
 */
extern s32 D_80181B98[];

/* Animation byte column of the shared work-record table, 0x98-byte records
 * based at 0x80146888: the byte at 0x801468FC is field 0x74 of a record. This
 * body raises bit 0 of that byte for the record the range-0 slot search
 * reports, and the original multiplies the record slot by 0x98 against this
 * column base instead of adding a row offset to the table base, so the column
 * keeps an address-derived symbol of its own.
 * @source 0x801468FC @kind unknown
 */
extern u8 D_801468FC[];

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses two row fields through their
 * own absolute symbols, because the original re-materialises each field base
 * and adds the scaled row offset again instead of keeping one row base.
 * @source 0x80143FD0 @kind unknown
 */
extern u8 D_80143FD0[];
/* Row field of the same 0x74-byte front-end row as D_80143FD0, two bytes on.
 * @source 0x80143FD2 @kind unknown
 */
extern u8 D_80143FD2[];

/* Cue dispatcher of the shared EXE image, outside this payload. The step
 * handlers of PLP012, PLP015, PLP025 and PLP247 declare the same prototype and
 * pass cue ids of the same kind: a work halfword plus 0x100 here, 0x10B here.
 */
void func_8015DF18(u32 arg0);

/* Readiness poll helper of the shared main executable, outside this payload.
 * This body calls it last on every path and discards the returned byte. */
extern u8 func_8014DAEC(void);

/* No-argument slot search of the shared EXE image, outside this payload: it
 * projects the same active work area with the undoubled offsets of
 * 0x80181B94/0x80181B98 and walks the 0x74-byte front-end rows at 0x80143FC8
 * whose byte 0 reads nonzero and whose byte 5 reads 0x17. It answers the signed
 * slot of the first row it accepts, or -1 when none is. This body seeds the row
 * that search reports and records the slot, so the answer is read as a signed
 * byte and tested against -1. The same prototype served the exact sibling
 * queueCue2CPlus100ThenSeedFrontEndRowFoundBySlotSearchElseClaimProjectedCellAndAdvanceWorkByte3PlcharPlp02600_801CFCD8
 * of PLP026 at 0x801CFCD8.
 */
s8 func_801B5AE4(void);

/* Range slot search of the same shared EXE image, outside this payload: it
 * walks the 0x74-byte front-end rows whose byte 5 reads 0x30 and answers the
 * signed slot of the first row that holds the supplied point at the supplied
 * range, or -1 when no row does. This body passes the projected point at range
 * 0 and raises the animation bit of the answered row. This target generated
 * undefined_funcs_auto.txt binds the address, and the exact sibling
 * queueCue2CPlus100ThenSeedFrontEndRowFoundBySlotSearchElseClaimProjectedCellAndAdvanceWorkByte3PlcharPlp26700_801CEFB0
 * declares the same prototype.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Map-code claim handler of this same payload, the asm row at 0x801CEE44: it
 * sign-extends its two signed halves, probes the map cell code through the
 * shared func_80166CB0 and claims a front-end row for the codes it accepts
 * through claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp46700_801CF050,
 * comparing the probe against 0xF4 first. It answers one for the accepted codes
 * and zero otherwise, and this body tests only the low byte of that answer, so
 * the declared return is that byte. Both cell offsets are signed sixteen-bit
 * values, so the original converts each retry index to that width before the
 * call. Its map row is already in this target symbols.txt, so the link needs no
 * new binding. The exact siblings of PLP016, PLP267 and PLP567 declare their own
 * payload claim handler with the same two-argument prototype and the same
 * low-byte answer test.
 */
u8 func_801CEE44(s16 col, s16 row);

/* @source 0x801CE9D4
 * @behavior Work byte 3 == 1 step of this overlay, the word 0x801CE9D4 at
 *           0x801D03D0, index 1 of the eight-word code-pointer run based at
 *           0x801D03CC that this target byte-3 dispatcher 0x801CE890 indexes
 *           with work byte 3 scaled by four, and entry 9 of the
 *           twenty-eight-word handler run based at 0x801D03AC, so no jal in this
 *           payload names it. While work byte 0x0A reads zero the body leaves
 *           the record untouched; otherwise it decrements that byte and
 *           continues only when the new value reads zero. That path first
 *           queues the cue computed from the work record unsigned halfword 0x2C
 *           plus 0x100 through the shared cue dispatcher func_8015DF18 and asks
 *           the no-argument slot search func_801B5AE4 for a front-end row. When
 *           that search answers a slot rather than -1 the step queues the
 *           literal cue 0x10B, writes 1 into field 0x0A and the work route byte
 *           0x08 into field 0x08 of that row, whose stride is 0x74, copies the
 *           answered slot into work byte 0x06 through the pointer cell, and
 *           advances work byte 3. Otherwise the step projects the work position
 *           by adding the doubled movement offsets of 0x80181B94 and 0x80181B98
 *           selected by the route byte 0x08 to the work words 0x34 and 0x38,
 *           hands that pair with range 0 to the shared slot search
 *           func_801BDE14 and, unless that search answers -1, raises bit 0 of
 *           the animation byte 0x74 of the found work record through the column
 *           base 0x801468FC and the stride 0x98 and queues the literal cue
 *           0x10B, then claims a front-end row at the projected point through
 *           this payload map-code claim handler func_801CEE44, passing the high
 *           half of each projected word as a signed half, retrying one unit
 *           further on the first axis when that claim fails and the first word
 *           has a nonzero low half and one unit further on the second axis when
 *           the retry also fails and the second word has a nonzero low half.
 *           Every path then increments work byte 3, so the accepted-row arm
 *           advances it twice, and the body closes by running the shared
 *           readiness helper func_8014DAEC. It takes no arguments and returns
 *           nothing. The 0x30-byte frame keeps $ra at 0x28 and the six saved
 *           registers across the calls, the range constant 0 is materialised in
 *           the delay slot of the branch into the projected path, and the cell
 *           load that opens the body issues before the frame is created. The
 *           roles of work bytes 0x03, 0x06, 0x08 and 0x0A, of the two front-end
 *           row fields, of the cues and of the two slot searches are not
 *           established, so the name describes the observed step and those stay
 *           described by address or by the effect the step has on them.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Seeded from the exact lift of the same body at PLP267 0x801CEFB0, which had
 * gated exact when this seed was written, and taken over with this payload own
 * map-code claim handler. This 0x24C-byte span compared to that range over the
 * payload bytes with exactly four differing words: the j-format word at body
 * offset 0xF0 that names the shared merge label and the three jal words at 0xF8,
 * 0x120 and 0x148 that name this payload map-code claim handler 0x801CEE44,
 * every one of them shifted by exactly the delta between the two spans, so one
 * body, one statement order and one register allocation are proven while the
 * payload-internal call target leaves no byte-identical clone. A little-endian
 * scan of out/binaries for this 588-byte span finds it once, at its own place in
 * this payload, and six plchar payloads hold it with those four words as their
 * only differences. The countdown byte 0x0A, the route byte 0x08, the step
 * counter byte 0x03, the slot byte 0x06, the halfword 0x2C and the position
 * offsets 0x34 and 0x38 are this span bytes.
 */
void queueCue2CPlus100ThenSeedFrontEndRowFoundBySlotSearchElseClaimProjectedCellAndAdvanceWorkByte3PlcharPlp46700_801CE9D4(
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
        claimed = func_801CEE44(cx, cy);
        if ((claimed & 0xFF) == 0) {
          if ((x & 0xFFFF) != 0) {
            claimed = func_801CEE44(cx + 1, cy);
          }
          if ((claimed & 0xFF) == 0 && (y & 0xFFFF) != 0) {
            func_801CEE44(cx, cy + 1);
          }
        }
      }
      D_1F800044[0x03] += 1;
    }
  }
  func_8014DAEC();
}
