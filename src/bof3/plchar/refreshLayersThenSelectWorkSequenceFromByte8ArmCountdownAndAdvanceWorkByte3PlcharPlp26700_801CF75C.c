#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads the cell before the
 * byte-8 read and again before each of the two stores, so the source keeps the
 * named-cell form instead of caching the pointer in a local.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Layer refresh of this payload. The original calls it through a plain `jal`
 * with nothing set up in the argument registers, so it takes no arguments and
 * this caller reads no result from $v0. It writes 1 into work byte 0x2B, probes
 * the two layer tables through 0x80154F78/0x80154F28 against the work reference
 * halfword 0x3E and clears that byte again when a probe reads above 64 and
 * beyond the reference; its 0x164-byte body is the helper the same-body step
 * handlers of PLP016 (0x801D0444) and PLP056 (0x801D066C) call, and its own row
 * here is still `asm`.
 */
void func_801CFB40(void);

/* Work-sequence selector of the shared EXE image, outside this payload. The
 * sibling step handlers at 0x801D0060 (PLP016), 0x801D0288 (PLP056) and
 * 0x801D0484 (PLP026) declare the same prototype and pass a sequence id, so the
 * call here passes a small immediate. Splat binds the address through this
 * target's generated undefined_funcs_auto.txt.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF75C
 * @behavior Work byte 3 == 0 step of this overlay: it is the word this
 *           payload's thirty-entry code-pointer run at 0x801D0988 holds at
 *           0x801D09C4, entry 15, and that word is the table base the byte-3
 *           dispatcher 0x801CF718 indexes by work byte 3, so the dispatcher
 *           reaches this body with index zero and the body has no direct caller
 *           of its own. It first calls the payload layer refresh at 0x801CFB40,
 *           then takes the work record published at the scratchpad pointer cell
 *           0x1F800044, reads that record's unsigned byte 8 and selects the
 *           shared work sequence through func_8014D8D4 with
 *           ((byte 8 - 1) / 2 + 0x42) truncated to a byte. It then writes 0xB
 *           into work byte 0xA, the counter the step-1 handler at 0x801CF7C8
 *           decrements, and 1 into work byte 3, which advances the dispatch to
 *           entry 16 of the same run. The original re-loads the pointer cell
 *           before the byte-8 read and before each of the two stores. The
 *           subtraction leaves the division signed, so byte 8 == 0 still selects
 *           sequence 0x42 while byte 8 == 1 and byte 8 == 2 both give 0x42
 *           and byte 8 == 3 gives 0x43. It takes no arguments, returns
 *           nothing, and the 0x18-byte frame
 *           only keeps $ra at 0x10 across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshLayersThenSelectWorkSequenceFromByte8ArmCountdownAndAdvanceWorkByte3PlcharPlp26700_801CF75C(
    void) {
  func_801CFB40();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}
