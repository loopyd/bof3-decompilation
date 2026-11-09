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

/* Animation byte column of the shared work-record table, 34 records of 0x98
 * bytes based at 0x80146888: the byte at 0x801468FC is field 0x74 of a record,
 * the animation halfword low byte whose bit 0 this body raises for the record
 * that holds the projected position. The original multiplies the record slot by
 * 0x98 against this column base instead of adding a row offset to the table
 * base, so the column keeps an address-derived symbol of its own. This target
 * generated undefined_syms_auto.txt binds the address.
 * @source 0x801468FC @kind unknown
 */
extern u8 D_801468FC[];

/* Slot search of the shared EXE image, outside this payload: it answers the
 * signed slot of the first eligible 0x98-byte work record that holds the
 * supplied point at the supplied range, or -1 when no record does. This target
 * generated undefined_funcs_auto.txt binds the address, and the exact sibling
 * markWorkAreaAnimationBitAndClaimRecordAtProjectedPositionThenAdvanceStepWhenWorkCountdownByteAExpiresPlcharPlp57800_801CEFC4
 * declares the same prototype.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Map-code step of this same payload, still an asm row at 0x801CFF14: its
 * prologue sign-extends both arguments through sll 16 plus sra 16, probes the
 * map cell code through the shared func_80166CB0 and claims a front-end record
 * through claimFrontEndRecordHandler34FromPositionPairStoreStateDistanceAndClearRowByteBPlcharPlp46700_801D010C
 * for the codes it accepts, comparing the probe against 0xF2 first. It answers
 * one for those codes and zero otherwise, and this body tests only the low byte
 * of that answer. Both cell offsets are signed sixteen-bit values, so the
 * original converts each retry index to that width before the call. Its map row
 * is already in this target symbols.txt, so the link needs no new binding.
 */
u8 func_801CFF14(s16 col, s16 row);

/* Shared readiness helper of the shared EXE image, outside this payload. This
 * body calls it last on every path and discards the returned byte. */
extern u8 func_8014DAEC(void);

/* @source 0x801CFC8C
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as word
 *           0x801D03FC, entry 20 of the handler run that starts at 0x801D03AC
 *           and that this overlay places as the textbin row at that base, and no
 *           jump word of this payload names the body. It decrements the work
 *           object countdown byte 0x0A and does nothing else while that byte
 *           stays nonzero. When it reaches zero the step projects the work
 *           position: the work words 0x34 and 0x38 are offset by the movement
 *           offsets of 0x80181B94 and 0x80181B98 selected by the route byte 0x08
 *           and doubled. It hands that pair with range 0 to the slot search
 *           func_801BDE14 and, unless the search answers -1, raises bit 0 of the
 *           animation byte 0x74 of the found work record through the column base
 *           0x801468FC and the slot stride 0x98. It then claims a map cell record
 *           at the projected point through this payload map-code step
 *           func_801CFF14, passing the high half of each projected word as a
 *           signed half: when that claim fails and the first word has a nonzero
 *           low half it retries one unit further on that axis, and when the retry
 *           also fails and the second word has a nonzero low half it retries one
 *           unit further on the second axis. The projected second word keeps a
 *           second live copy because the two cell halves claim the register that
 *           holds it, so the last retry test reads that copy and the source binds
 *           it to a variable of its own. Both paths then increment the step
 *           counter byte 0x02 through the scratchpad cell itself, and the body
 *           closes by running the shared readiness helper func_8014DAEC on every
 *           path. It takes no arguments and returns nothing. The 0x30-byte frame
 *           keeps $ra at 0x28 and the six saved registers across the calls, the
 *           range constant is materialised in the delay slot of the branch that
 *           leaves the expiry path, and the increment of byte 0x02 issues before
 *           the jump to the shared call.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Seeded from the lift of the same body at plp578 0x801CEFC4, which had gated
 * exact when this seed was written. This 0x19C-byte span compared to that range
 * over the payload bytes with exactly three differing words, all of them the
 * jump word at body offsets 0xF8, 0x120 and 0x148 that names this payload map
 * code step 0x801CFF14 (0x0C073FC5) where the plp578 copy names its own claim
 * handler 0x801CF24C (0x0C073C93); the remaining one hundred words stay equal,
 * so one body and one statement order are proven while the payload-internal call
 * target leaves no byte-identical clone. The same span also sits word-identical
 * apart from that one jump word in sixteen other plchar payloads, so the plp578
 * source supplied the statement order and this body supplied only its own claim
 * call. Four of the constants of the body are its own: the countdown byte 0x0A,
 * the route byte 0x08, the step counter byte 0x02 and the offsets 0x34 and 0x38
 * of the position pair, all read from this span bytes.
 */
void markWorkAreaAnimationBitAndClaimRecordAtProjectedPositionThenAdvanceStepWhenWorkCountdownByteAExpiresPlcharPlp46700_801CFC8C(
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
    claimed = func_801CFF14(cx, cy);
    if ((claimed & 0xFF) == 0) {
      if ((x & 0xFFFF) != 0) {
        claimed = func_801CFF14(cx + 1, cy);
      }
      if ((claimed & 0xFF) == 0 && (y2 & 0xFFFF) != 0) {
        func_801CFF14(cx, cy + 1);
      }
    }
    D_1F800044[0x02] += 1;
  }
  func_8014DAEC();
}
