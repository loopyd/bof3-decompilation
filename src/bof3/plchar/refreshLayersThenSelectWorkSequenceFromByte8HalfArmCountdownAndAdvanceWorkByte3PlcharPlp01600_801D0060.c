#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it before the
 * record-byte read and again before each of the two stores, so the source keeps
 * the named symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Claim-byte refresh of this payload; the original calls it through a plain
 * `jal` with nothing set up in the argument registers, so it takes no
 * arguments. Its own lift at 0x801D0444 came out exact, and this caller reads no
 * result from $v0.
 */
void claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp01600_801D0444(
    void);

/* Work-sequence selector of the shared EXE image, outside this payload. The ui
 * and bmagic lifts (for example src/bof3/ui/selectWorkSequenceForRoute7.c)
 * declare the same prototype and pass a sequence id; Splat binds the address
 * through this target's generated undefined_funcs_auto.txt, and the remaining
 * asm rows of this payload reference it too.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801D0060
 * @behavior Work byte 3 == 0 step of this overlay: entry 0 of the work-byte-3
 *           handler run based at 0x801D0684, whose entries 1 and 2 are
 *           0x801D00CC and 0x801D0204 (the run itself starts at 0x801D0630 and
 *           is the `textbin` row T_801D0630). It first calls the payload
 *           helper at 0x801D0444, then takes the work record published at the
 *           scratchpad pointer cell 0x1F800044, reads that record's unsigned
 *           byte 8 and selects the shared work sequence through func_8014D8D4
 *           with ((byte 8 - 1) / 2 + 0x42) truncated to a byte. It then writes
 *           0xB into work byte 0xA, the counter the step-1 handler at
 *           0x801D00CC decrements, and 1 into work byte 3, which advances the
 *           dispatch to entry 1 of the same run. The original re-loads the
 *           pointer cell before the byte-8 read and before each of the two
 *           stores. The subtraction leaves the division signed, so byte 8 == 0
 *           still selects sequence 0x42. It takes no arguments, returns
 *           nothing, and the 0x18-byte frame only keeps $ra at 0x10 across the
 *           helper call. Its 108-byte body is byte-identical to func_801D0484
 *           of PLP026 except for the helper the first `jal` reaches (0x801D0868
 *           there), so that sibling is shape evidence, not a clone, and its own
 *           row is still `asm`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshLayersThenSelectWorkSequenceFromByte8HalfArmCountdownAndAdvanceWorkByte3PlcharPlp01600_801D0060(
    void) {
  claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp01600_801D0444();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}
