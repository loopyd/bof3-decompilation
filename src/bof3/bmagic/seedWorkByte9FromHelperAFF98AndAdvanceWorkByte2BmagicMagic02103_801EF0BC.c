#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map under the name the exact bmagic siblings use. The cell is read once per
 * access, so the original re-materializes its address (lui $v1,0x1F80 /
 * lw $v1,68($v1)) both for the work byte 9 store after the call and for the
 * work byte 2 increment. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Out-of-payload helper the payload only calls; it returns the byte this
 * handler stores in work byte 9 and consumes no arguments. */
extern u8 func_800AFF98(void); /* @source 0x800AFF98 @kind function */

/* @source 0x801EF0BC
 * @behavior Entry 0 of the six-word code pointer run at 0x801EEC10
 *           (0x801EF0BC, 0x801EF104, 0x801EF208, 0x801EF260, 0x801EF2CC,
 *           0x801EF328), which the neighbouring dispatcher at 0x801EF020 copies
 *           into its frame and indexes with the work object's byte 2. Calls the
 *           helper at 0x800AFF98 with no arguments, stores the returned byte in
 *           scratchpad work byte 9, then reloads the pointer cell at 0x1F800044
 *           and increments work byte 2 by one at byte width, selecting run entry
 *           1. Payload word 0x801EEC10 is the only in-image reference to this
 *           address and no jal targets it. No other field is read or written,
 *           nothing is returned, and the 0x18-byte frame only keeps $ra at 0x10
 *           across the call. The roles of work bytes 9 and 2 beyond that
 *           dispatch use stay offset-anchored.
 *
 *           The 72-byte payload range is byte-identical to the exact lifts
 *           0x801EF024 of bmagic/magic161/03, 0x801EF9A0 of bmagic/magic004/03,
 *           0x801EF544 of bmagic/magic154/03, 0x801EF2E0 of bmagic/magic065/00
 *           and 0x801F133C of bmagic/magic008/00 (one payload cmp each); every
 *           twin encodes the same scratchpad cell and the same absolute callee,
 *           so no word shifts between the spans and the body is transcribed
 *           from them without a rewrite.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2BmagicMagic02103_801EF0BC(void) {
  u8 value;

  value = func_800AFF98();
  g_battle_work[9] = value;
  g_battle_work[2]++;
}
