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

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses two row fields through their own
 * absolute symbols, because the original re-materialises each field base and
 * adds the scaled row offset again instead of keeping one row base. Row byte
 * 0x08 of that table holds a work route byte and row byte 0x0A the flag this
 * step raises.
 * @source 0x80143FD0 @kind unknown
 */
extern u8 D_80143FD0[];
/* Row field of the same 0x74-byte front-end row as D_80143FD0, two bytes on.
 * @source 0x80143FD2 @kind unknown
 */
extern u8 D_80143FD2[];

/* Cue dispatcher of the shared main executable, outside this payload. This body
 * passes the literal cue 0x10B and discards any result; its address resolves
 * from this target's generated undefined funcs list.
 */
extern void func_8015DF18(u32 cue);

/* Readiness poll helper of the main executable, outside this payload. This body
 * calls it once on every path, as its last statement, and discards the answer.
 */
extern u8 func_8014DAEC(void);

/* No-argument slot search of the shared main executable, outside this payload:
 * it answers the signed slot of the first eligible 0x74-byte front-end row of
 * the table at 0x80143FC8, or -1 when no row is. The original sign-extends the
 * answered byte and tests it against -1.
 */
s8 func_801B5AE4(void);

/* Range slot search of the same shared image, outside this payload: it answers
 * the signed slot of the first eligible 0x98-byte work record that holds the
 * supplied point at the supplied range, or -1 when no record does. This body
 * passes the projected point at range 0, and the original sign-extends the
 * answered byte.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Map cell claim of this same payload, still an asm row at 0x801CF940 in this
 * target: it sign-extends both arguments through sll plus sra in its own
 * prologue, probes the map cell code at the supplied signed column and row and
 * claims a front-end row for the codes it handles, answering one when a row was
 * claimed and zero otherwise. It reads no third argument register, so the two
 * coordinates are its whole argument list, and this body tests only the low byte
 * of its answer.
 */
u8 func_801CF940(s16 col, s16 row);

/* @source 0x801CF4E0
 * @behavior Work byte 3 equal to 1 step of this overlay's handler run. The body
 *           address appears once in the payload, as the word 0x801CF4E0 at
 *           0x801D0324, entry 12 of the twenty-eight-word code-pointer run based
 *           at 0x801D02F4 and index 1 of the run based at 0x801D0320 that the
 *           byte-3 dispatcher dispatchWorkByte3Table0320PlcharPlp03400_801CF37C
 *           indexes from that base with work byte 3 scaled by four, so no jal in
 *           this payload names the body and the run is its only in-image
 *           reference. It reads the front-end work object published at the
 *           scratchpad pointer cell 0x1F800044 and decrements its countdown byte
 *           0x0A; while that byte stays nonzero the body writes nothing and only
 *           runs the shared readiness helper func_8014DAEC at its end. When the
 *           countdown reaches zero the step asks the no-argument slot search
 *           func_801B5AE4 for a front-end row. An answered slot makes the step
 *           seed that row of the shared 0x74-byte front-end table: it writes 1
 *           into row byte 0x0A and the work route byte 0x08 into row byte 0x08,
 *           then queues the literal cue 0x10B through the shared cue dispatcher
 *           func_8015DF18, copies the answered slot into work byte 0x06 and
 *           advances work byte 3. A -1 answer makes the step project the work
 *           position by adding the doubled movement offsets of 0x80181B94 and
 *           0x80181B98 selected by the route byte 0x08 to work words 0x34 and
 *           0x38, hand that pair to the range-0 slot search func_801BDE14 and,
 *           unless the search answers -1, raise bit 0 of the animation byte at
 *           0x801468FC plus the found slot scaled by the 0x98-byte work record
 *           stride and queue the same literal cue 0x10B. It then claims a
 *           front-end row at the projected point through this payload's own map
 *           cell claim func_801CF940, passing the high half of each projected
 *           word: when that claim answers zero and the first word has a nonzero
 *           low half it retries one unit further on that axis, and when the retry
 *           also answers zero and the second word has a nonzero low half it
 *           retries one unit further on the second axis, discarding the last
 *           answer. Every path then advances work byte 3 once more, so the
 *           seeded-row arm advances it twice, and the body closes by running
 *           func_8014DAEC. It takes no arguments and returns nothing. The
 *           pointer-cell load that opens the body issues before frame creation,
 *           the 0x30-byte frame keeps the return address at 0x28 and the six
 *           saved registers across the calls, the range constant 0 is
 *           materialised at the head of the projected arm, and on the seeded-row
 *           arm the increment of work byte 3 issues in the delay slot of the jump
 *           to the shared tail that advances it the second time. The roles of
 *           work bytes 0x03, 0x06, 0x08 and 0x0A, of the two front-end row
 *           fields, of the cue 0x10B and of the two slot searches are not
 *           established, so they stay described by address or by the effect this
 *           step has on them. Shape evidence: a word scan of the 394 extracted
 *           payloads for this 0x23C-byte span found one twin, the still-asm row
 *           func_801CE7B8 of PLP349, whose 143 words are this body with only its
 *           one jump word and three call words differing, each by exactly the
 *           0xD28 offset between the two spans, which proves one body and leaves
 *           that twin without a source to clone. The statement order and
 *           declaration style therefore follow the already-lifted cue-then-seed
 *           step at 0x801CFCD8 of PLP026, which is this body plus a leading cue
 *           call built from the work halfword 0x2C and with the literal cue 0x10B
 *           queued before the row seed instead of after it; that lift was still
 *           review-pending at 89.80 percent when this seed was written, so it was
 *           used as shape evidence only.
 * @status partial
 * @match 89.51
 * @residual 128/143 instructions and 572 of 572 bytes: the emitted instruction
 *           sequence, order and size are the original, and every one of the
 *           fifteen differing instructions is a register operand of an
 *           otherwise identical opcode, across two spans. This compiler build
 *           colours the searched slot byte and its sign-extended copy into $s1
 *           and $s0 where the original uses $s0 and $s1, and then colours the
 *           projected first word and its high half into $s1 and $s2 where the
 *           original uses $s2 and $s1, which is the same mirror that the lifts
 *           of the byte-identical bodies at 0x801CF8B4 of PLP016 and 0x801CFCD8
 *           of PLP026 measured at 89.80 percent. Five source spellings of that
 *           pair were measured here and all five produced the same mirror: one
 *           signed local, a separate unsigned copy for the work byte store, one
 *           unsigned local with a signed index, a store through an unsigned
 *           cast, and explicit signed casts at the comparison and at the scale.
 *           This body own bytes fix both live ranges and both reference counts,
 *           so the smallest missing evidence is the register-assignment
 *           behaviour of the original compiler build for this shape, not a
 *           further spelling of the source.
 */
void tickWorkCountdownByteAThenSeedFrontEndRowFoundBySlotSearchAndQueueCue10bElseMarkProjectedRecordClaimProjectedCellAndAdvanceWorkByte3PlcharPlp03400_801CF4E0(
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
      slot = func_801B5AE4();
      if (slot != -1) {
        off = slot * 0x74;
        route = D_1F800044[0x08];
        D_80143FD2[off] = 1;
        D_80143FD0[off] = route;
        func_8015DF18(0x10B);
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
        claimed = func_801CF940(cx, cy);
        if (claimed == 0) {
          if ((x & 0xFFFF) != 0) {
            claimed = func_801CF940(cx + 1, cy);
          }
          if (claimed == 0 && (y & 0xFFFF) != 0) {
            func_801CF940(cx, cy + 1);
          }
        }
      }
      D_1F800044[0x03] += 1;
    }
  }
  func_8014DAEC();
}
