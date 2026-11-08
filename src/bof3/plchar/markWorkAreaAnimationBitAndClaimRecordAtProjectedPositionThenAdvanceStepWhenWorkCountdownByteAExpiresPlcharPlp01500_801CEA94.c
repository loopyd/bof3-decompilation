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

/* Slot search of the shared EXE image, outside this payload: it answers the
 * signed slot of the first eligible 0x98-byte work record that holds the
 * supplied point at the supplied range, or -1 when no record does. The sibling
 * setResolvedSlotAnimBit0WhenFacingMatchesElseFaceAndDecrementByte9World03Area12114_801F50F4
 * declares the same prototype.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Record claim of this same payload, still `asm` in this target: it sign-extends
 * both arguments through `sll` plus `sra` in its own prologue, asks the shared
 * allocator func_8019601C for a front-end record and answers one when the claim
 * succeeded and zero when it did not. The lifted
 * claimFrontEndRecordHandler34FromPositionPairStoreStateDistanceAndClearRowByteBPlcharPlp01500_801CEF14
 * is its callee, and the exact sibling
 * markWorkAreaAnimationBitAndClaimRecordAtProjectedPositionThenAdvanceStepWhenWorkCountdownByteAExpiresPlcharPlp27a00_801CF1C0
 * declares the same pair-of-signed-cells prototype for its own payload's claim.
 */
u8 func_801CED1C(s16 x, s16 y);

/* Shared readiness helper of the shared EXE image, outside this payload. This
 * body calls it on both paths and discards the returned byte. */
u8 func_8014DAEC(void);

/* @source 0x801CEA94
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as word
 *           0x801CFB94, entry 4 of the sixteen-word handler run that starts at
 *           0x801CFB84. It decrements the work object's countdown byte 0x0A and
 *           does nothing else while that byte stays nonzero. When it reaches
 *           zero the step projects the work position: the work words 0x34 and
 *           0x38 are offset by the movement offsets of 0x80181B94 and 0x80181B98
 *           selected by the route byte 0x08 and doubled. It hands that pair to
 *           the slot search func_801BDE14 with range 0 and, unless the search
 *           answers -1, raises bit 0 of the animation byte 0x74 of the found
 *           work record through the column base 0x801468FC and the slot stride
 *           0x98. It then claims a front-end record at the projected point
 *           through func_801CED1C, passing the high half of each projected word:
 *           when that claim fails and the first word has a nonzero low half it
 *           retries one unit further on that axis, and when the retry also fails
 *           and the second word has a nonzero low half it retries one unit
 *           further on the second axis. The projected second word keeps a second
 *           live copy because the two cell halves claim the register that holds
 *           it, so the last retry test reads that copy and the source binds it to
 *           a variable of its own. Both paths then increment the step counter
 *           byte 0x02 through the scratchpad cell itself, and the body closes by
 *           running the shared readiness helper func_8014DAEC on every path. It
 *           takes no arguments and returns nothing. The 0x30-byte frame keeps
 *           $ra at 0x28 and the six saved registers across the calls, the range
 *           constant is materialised in the delay slot of the branch that leaves
 *           the expiry path, and the increment of byte 0x02 issues before the
 *           jump to the shared call. The span at payload offset 0x694 is
 *           byte-identical to the span at the same offset of the plp012 and
 *           plp016 overlays, whose rows there are still asm, so those copies are
 *           evidence of a shared handler and not a lift of this target.
 * @status exact
 * @match 100.00
 * @residual none
 */
void markWorkAreaAnimationBitAndClaimRecordAtProjectedPositionThenAdvanceStepWhenWorkCountdownByteAExpiresPlcharPlp01500_801CEA94(
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
    claimed = func_801CED1C(cx, cy);
    if ((claimed & 0xFF) == 0) {
      if ((x & 0xFFFF) != 0) {
        claimed = func_801CED1C(cx + 1, cy);
      }
      if ((claimed & 0xFF) == 0 && (y2 & 0xFFFF) != 0) {
        func_801CED1C(cx, cy + 1);
      }
    }
    D_1F800044[0x02] += 1;
  }
  func_8014DAEC();
}
