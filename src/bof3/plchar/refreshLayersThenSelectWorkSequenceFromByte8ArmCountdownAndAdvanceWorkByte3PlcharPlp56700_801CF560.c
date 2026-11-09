#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it before the
 * record-byte read and again before each of the two stores, so the source keeps
 * the named symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Claim-byte refresh of this payload at 0x801CF944, lifted exact as
 * claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp56700_801CF944
 * when this declaration was re-pointed. Its body claims work byte 0x2B, probes
 * two layer positions against the work reference halfword 0x3E and clears the
 * claim again when a probe reads above 64. The original calls it through a plain
 * `jal` with nothing set up in the argument registers, so it takes no arguments,
 * and this caller reads no result from $v0.
 */
void claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp56700_801CF944(
    void);

/* Work-sequence selector of the shared EXE image, outside this payload. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF560
 * @behavior Work byte 3 == 0 step of this overlay: the word at 0x801D07CC is
 *           the table base the byte-3 dispatcher 0x801CF51C indexes by work
 *           byte 3, so that dispatcher reaches this body with index zero and the
 *           body has no direct caller of its own. It first calls the payload
 *           claim-byte refresh at 0x801CF944, then takes the work record
 *           published at the scratchpad pointer cell 0x1F800044, reads that
 *           record's unsigned byte 8 and selects the shared work sequence
 *           through func_8014D8D4 with ((byte 8 - 1) / 2 + 0x42) truncated to a
 *           byte. It then writes 0xB into work byte 0xA, the counter the step-1
 *           handler at 0x801CF5CC decrements, and 1 into work byte 3, which
 *           advances the dispatch to entry 1 of the same run at 0x801D07D0. The
 *           original re-loads the pointer cell before the byte-8 read and before
 *           each of the two stores. The subtraction leaves the division signed,
 *           so byte 8 == 0, 1 and 2 all select sequence 0x42 while byte 8 == 3
 *           gives 0x43. It takes no arguments, returns nothing, and the 0x18-byte
 *           frame only keeps $ra at 0x10 across the two calls. Its 0x6C-byte
 *           body differed from the already exact lift of the same body at PLP016
 *           0x801D0060 in exactly its first `jal` word when this seed was
 *           written, so that sibling is shape evidence, not a clone, and its own
 *           row was `asm`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshLayersThenSelectWorkSequenceFromByte8ArmCountdownAndAdvanceWorkByte3PlcharPlp56700_801CF560(
    void) {
  claimWorkByte2BThenClearItWhenRouteThreeOrFiveSampleAbove64AndBeyondWorkReferencePlcharPlp56700_801CF944();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}
