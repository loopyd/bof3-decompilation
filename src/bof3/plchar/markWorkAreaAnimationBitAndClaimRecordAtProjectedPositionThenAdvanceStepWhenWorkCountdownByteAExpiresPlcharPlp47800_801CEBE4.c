#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads the cell once for the countdown store, re-reads it for the
 * expiry condition and the whole expiry path, and loads it once more for the
 * closing step counter store, because a store through the pointer may alias the
 * cell itself, so the source keeps the named-symbol form and evaluates it again
 * at every access.
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

/* Slot search of the shared EXE image, outside this payload: it answers the
 * signed slot of the first eligible 0x98-byte work record that holds the
 * supplied point at the supplied range, or -1 when no record does. This
 * target's generated undefined_funcs_auto.txt binds the address, and the
 * original passes the range constant 0 as the third argument.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Map-cell claim handler of this payload, still `asm` in this target at
 * 0x801CEE6C: it sign-extends both arguments through `sll` plus `sra` in its own
 * prologue, asks the shared map probe func_80166CB0 for the one-byte code of
 * that cell, claims a front-end record for the codes it handles and answers one
 * when a record was claimed and zero otherwise. It is a boundary of its own, so
 * its name stays address-anchored here and this body only calls it.
 */
u8 func_801CEE6C(s16 x, s16 y);

/* Shared readiness helper of the shared EXE image, outside this payload. This
 * body calls it on both paths and discards the returned byte. */
u8 func_8014DAEC(void);

/* @source 0x801CEBE4
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as word
 *           0x801CEBE4 at 0x801CF6F4, entry 7 of the twenty-word code pointer
 *           run based at 0x801CF6D8. This overlay's byte-2 work dispatcher
 *           indexes that run from its own base 0x801CF6F0, where entry 7 sits
 *           at index 1, so the loader that steps the run on work byte 2 equal
 *           to 1 installs this body; it has no `jal` caller in the image. It
 *           decrements the countdown byte 0x0A of the work object and, while
 *           that byte stays nonzero, only runs the shared readiness helper
 *           func_8014DAEC. When the byte reaches zero the step projects the work
 *           position: it reads route byte 0x08, scales it by eight and adds the
 *           doubled movement offsets of 0x80181B94 and 0x80181B98 to work words
 *           0x34 and 0x38. It hands that pair with range 0 to the shared slot
 *           search func_801BDE14 and, unless the search answers -1, raises bit 0
 *           of the animation byte at 0x801468FC plus the found slot scaled by
 *           the 0x98-byte work record stride. It then claims a front-end record
 *           at the projected point through this payload's own claim handler at
 *           0x801CEE6C, passing the high half of each projected word as signed
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
 *           returns nothing; the two argument registers the analyzer reports
 *           stay unwritten by any caller and are overwritten by the body before
 *           their first use. The 0x30-byte frame keeps $ra at 0x28 and the six
 *           saved registers across the calls, the range constant is materialised
 *           in the delay slot of the branch that leaves the expiry path, and the
 *           increment of byte 0x02 issues before the closing shared call.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape note from the original bytes: the 0x19C-byte body differs from the
 * exact plp247/00 lift of the span at 0x801CF3DC in exactly three words,
 * 0x801CECDC, 0x801CED04 and 0x801CED2C, the three call words that name the
 * claim handler; this payload places that handler at 0x801CEE6C instead of
 * 0x801CF664, one equal shift of -0x7F8 for all three. No copy of this span in
 * any overlay binary is byte-identical to this one, so the body was
 * transposed from the exact lift of that span rather than cloned, and its
 * statement order, operand forms, offset spelling and declaration style follow
 * that lift with this payload's own handler as the callee.
 */
void markWorkAreaAnimationBitAndClaimRecordAtProjectedPositionThenAdvanceStepWhenWorkCountdownByteAExpiresPlcharPlp47800_801CEBE4(
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
    claimed = func_801CEE6C(cx, cy);
    if ((claimed & 0xFF) == 0) {
      if ((x & 0xFFFF) != 0) {
        claimed = func_801CEE6C(cx + 1, cy);
      }
      if ((claimed & 0xFF) == 0 && (y2 & 0xFFFF) != 0) {
        func_801CEE6C(cx, cy + 1);
      }
    }
    D_1F800044[0x02] += 1;
  }
  func_8014DAEC();
}
