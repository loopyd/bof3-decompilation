#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads it ahead of frame creation for the countdown read, re-reads it
 * for the second countdown test and once more before each work byte store,
 * because a store through the pointer may alias the cell itself. The source
 * keeps the named symbol and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the active work area
 * route byte at offset 0x08. The original scales that byte by eight once, which
 * is element index route * 2, and doubles each offset before it adds it, so both
 * bases stay their own symbol.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32 D_80181B98[];

/* Animation byte column of the shared work-record table, 0x98-byte records based
 * at 0x80146888: the byte at 0x801468FC is field 0x74 of a record, the animation
 * halfword low byte whose bit 0 this body raises for the record that holds the
 * projected position. The original multiplies the record slot by 0x98 against
 * this column base instead of adding a row offset to the table base, so the
 * column keeps an address-derived symbol of its own.
 * @source 0x801468FC @kind unknown
 */
extern u8 D_801468FC[];

/* Front-end record table field bases of the shared main-executable data segment:
 * rows of 0x74 bytes based at 0x80143FC8. This body addresses two row fields
 * through their own absolute symbols, because the original re-materialises each
 * field base and adds the scaled row offset again instead of keeping one row
 * base. Row byte 0x08 of that table holds a work route byte and row byte 0x0A
 * the flag this step raises.
 * @source 0x80143FD0 @kind unknown
 */
extern u8 D_80143FD0[];
/* Row field of the same 0x74-byte front-end row as D_80143FD0, two bytes on.
 * @source 0x80143FD2 @kind unknown
 */
extern u8 D_80143FD2[];

/* Readiness poll helper of the shared EXE image, outside this payload. This body
 * calls it once on every path, as its last statement, and discards the answer.
 */
u8 func_8014DAEC(void);

/* Cue dispatcher of the shared EXE image, outside this payload. The original
 * passes the literal cue 0x10B and keeps no result.
 */
extern void func_8015DF18(u32 cue);

/* No-argument slot search of the shared main executable, outside this payload:
 * it answers the signed slot of the first eligible 0x74-byte front-end row of
 * the table at 0x80143FC8, or -1 when no row is. The body keeps the answered
 * byte as an unsigned copy for the work byte store and sign-extends a second
 * copy for the -1 test, the front-end row offset and the record comparison of
 * the projected arm.
 */
s8 func_801B5AE4(void);

/* Range slot search of the same shared image, outside this payload: it answers
 * the signed slot of the first eligible 0x98-byte work record that holds the
 * supplied point at the supplied range, or -1 when no record does. This body
 * passes the projected point at range 0.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Map cell claim of this same payload, carried by the exact lift at 0x801CEC18
 * of this target: it sign-extends both arguments through sll plus sra in its
 * own prologue, probes the map cell code at the supplied signed column and row
 * through the shared func_80166CB0 and claims a front-end row for the codes it
 * handles, answering one when a row was claimed and zero otherwise. It reads no
 * third argument register, so the two coordinates are its whole argument list,
 * and this body tests only the low byte of its answer.
 */
u8 claimFrontEndRecordForMapCodesF0F1F4F6F7WithRandomStateElseStageItemNameStream29CueThenSetWorkByteBPlcharPlp34900_801CEC18(
    s16 col, s16 row);

/* @source 0x801CE7B8
 * @behavior Work byte 3 == 1 step of this overlay: the body address appears once
 *           in the payload's bytes, at the pointer word 0x801CF604, entry 6 of
 *           the sixteen-word code-pointer run the payload places at 0x801CF5EC.
 *           That word is four bytes past the base word 0x801CF600 that the
 *           byte-3 dispatcher dispatchWorkByte3Tablef600PlcharPlp34900_801CE654
 *           indexes with work byte 3 scaled by four, so the body runs as step 1
 *           of that dispatcher and no jal in this payload names it. It reads the
 *           front-end work object published at the scratchpad pointer cell
 *           0x1F800044 and, while its countdown byte 0x0A reads nonzero,
 *           decrements that byte and re-reads the cell; only a countdown that
 *           reaches zero runs the step, and every path closes by running the
 *           shared readiness helper func_8014DAEC. The step asks the
 *           no-argument slot search func_801B5AE4 for a front-end row. An
 *           answered slot seeds that row of the shared 0x74-byte front-end
 *           table, writing 1 into row byte 0x0A and the work route byte 0x08
 *           into row byte 0x08, queues the literal cue 0x10B through the shared
 *           cue dispatcher func_8015DF18, copies the answered byte into work
 *           byte 0x06 and advances work byte 3. A -1 answer projects the work
 *           position by adding the doubled movement offsets of 0x80181B94 and
 *           0x80181B98 selected by the route byte 0x08 to work words 0x34 and
 *           0x38, hands that pair to the range-0 slot search func_801BDE14 and,
 *           unless the search answers the same -1, raises bit 0 of the animation
 *           byte at 0x801468FC plus the found slot scaled by the 0x98-byte work
 *           record stride and queues the same cue 0x10B. It then claims a
 *           front-end row at the projected point through this payload's own map
 *           cell claim
 *           claimFrontEndRecordForMapCodesF0F1F4F6F7WithRandomStateElseStageItemNameStream29CueThenSetWorkByteBPlcharPlp34900_801CEC18,
 *           passing the high half of each projected word: when that claim
 *           answers zero and the first word has a nonzero
 *           low half it retries one unit further on that axis, and when the
 *           retry also answers zero and the second word has a nonzero low half
 *           it retries one unit further on the second axis, discarding the last
 *           answer. Every path then advances work byte 3 once more, so the
 *           seeded-row arm advances it twice, and the body closes by running
 *           func_8014DAEC. It takes no arguments and returns nothing. The
 *           pointer-cell load that opens the body issues before frame creation,
 *           the 0x30-byte frame keeps the return address at 0x28 and the six
 *           saved registers across the calls, the range constant 0 is
 *           materialised at the head of the projected arm, and on the
 *           seeded-row arm the increment of work byte 3 issues in the delay slot
 *           of the jump to the shared tail that advances it the second time. The
 *           roles of work bytes 0x03, 0x06, 0x08 and 0x0A, of the two front-end
 *           row fields, of the cue 0x10B and of the two slot searches are not
 *           established, so they stay described by address or by the effect this
 *           step has on them. Shape evidence: the 0x23C-byte span at 0x801CF4E0
 *           of the PLP034 payload is this body word for word apart from its one
 *           intra-body jump word and its three call words, each shifted by
 *           exactly the 0xD28 offset between the two spans, which one payload
 *           comparison of the two 143-word spans established, and that twin was
 *           already lifted but measured 89.51 percent, so its body is shape
 *           evidence only. The statement order, operand forms and declaration
 *           style instead follow the lift at 0x801CF8B4 of PLP016, which
 *           measured exact before this seed was written and is the same body plus
 *           one leading cue call built from work halfword 0x2C with the literal
 *           cue 0x10B queued before the row seed rather than after it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickWorkCountdownByteAThenSeedFrontEndRowFoundBySlotSearchAndQueueCue10bElseMarkProjectedRecordClaimProjectedCellAndAdvanceWorkByte3PlcharPlp34900_801CE7B8(
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
      answer = func_801B5AE4();
      signed_answer = (s8)answer;
      if (signed_answer != -1) {
        b8 = D_1F800044[0x08];
        D_80143FD2[(u32)signed_answer * 0x74u] = 1;
        D_80143FD0[(u32)signed_answer * 0x74u] = b8;
        func_8015DF18(0x10B);
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
        claimed =
            claimFrontEndRecordForMapCodesF0F1F4F6F7WithRandomStateElseStageItemNameStream29CueThenSetWorkByteBPlcharPlp34900_801CEC18(
                cx, cy);
        if ((claimed & 0xFF) == 0) {
          if ((x & 0xFFFF) != 0) {
            claimed =
                claimFrontEndRecordForMapCodesF0F1F4F6F7WithRandomStateElseStageItemNameStream29CueThenSetWorkByteBPlcharPlp34900_801CEC18(
                    cx + 1, cy);
          }
          if ((claimed & 0xFF) == 0 && (y & 0xFFFF) != 0) {
            claimFrontEndRecordForMapCodesF0F1F4F6F7WithRandomStateElseStageItemNameStream29CueThenSetWorkByteBPlcharPlp34900_801CEC18(
                cx, cy + 1);
          }
        }
      }
      D_1F800044[0x03] += 1;
    }
  }
  func_8014DAEC();
}
