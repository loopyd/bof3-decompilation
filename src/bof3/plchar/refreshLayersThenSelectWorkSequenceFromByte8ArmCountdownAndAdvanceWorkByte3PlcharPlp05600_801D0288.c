#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-loads the cell before the byte-8 read and again before each of the
 * two stores, so the source keeps the named-cell form instead of caching the
 * pointer in a local.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Layer refresh of this payload. The original calls it through a plain `jal`
 * with nothing set up in the argument registers, so it takes no arguments and
 * this caller reads no result from $v0. Its 0x164-byte body is byte-identical,
 * all 89 instructions, to the helper the two sibling step handlers call —
 * 0x801D0444 of PLP016 and 0x801D0868 of PLP026, the helper PLP026 describes as
 * refreshing the layers — and its own row here is a `c` lift of its own.
 */
void claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp05600_801D066C(
    void);

/* Work-sequence selector of the shared EXE image, outside this payload. The
 * sibling step handlers at 0x801D0060 (PLP016) and 0x801D0484 (PLP026) declare
 * the same prototype and pass a sequence id, so the two calls here pass a small
 * immediate. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt and the remaining asm rows of this payload reference
 * it too.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801D0288
 * @behavior Work byte 3 == 0 step of this overlay: entry 0 of the two-entry
 *           work-byte-3 handler run based at 0x801D08B0, whose entry 1 is
 *           0x801D02F4. That base is word 22 of the twenty-four-entry pointer
 *           run the Splat `textbin` row at 0x801D0858 places, and the byte-3
 *           dispatcher at 0x801D0244 reaches it by scaling work byte 3, so this
 *           body has no direct caller of its own. It first calls the payload
 *           layer refresh at 0x801D066C, then takes the work record published at
 *           the scratchpad pointer cell 0x1F800044, reads that record's unsigned
 *           byte 8 and selects the shared work sequence through func_8014D8D4
 *           with ((byte 8 - 1) / 2 + 0x42) truncated to a byte. It then writes
 *           0xB into work byte 0xA, the counter the step-1 handler at 0x801D02F4
 *           decrements, and 1 into work byte 3, which advances the dispatch to
 *           entry 1 of the same run. The subtraction leaves the division signed,
 *           so byte 8 == 0 still selects sequence 0x42. It takes no arguments,
 *           returns nothing, and the 0x18-byte frame only keeps $ra at 0x10
 *           across the two calls. Its 0x6C-byte body is byte-identical to
 *           func_801D0060 of PLP016 and func_801D0484 of PLP026 except for the
 *           helper the first `jal` reaches, so those siblings are shape
 *           evidence, not clones, and both of their rows are already `c`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshLayersThenSelectWorkSequenceFromByte8ArmCountdownAndAdvanceWorkByte3PlcharPlp05600_801D0288(
    void) {
  claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp05600_801D066C();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}
