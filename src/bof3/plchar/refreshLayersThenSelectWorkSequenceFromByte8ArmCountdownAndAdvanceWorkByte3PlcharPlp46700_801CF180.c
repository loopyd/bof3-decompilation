#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it before the
 * record-byte read and again before each of the two stores, so the source keeps
 * the named symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Layer-position refresh of this payload, still an `asm` row here. It claims
 * work byte 0x2B, draws the work reference positions and clears the claim again
 * when a probe reads above 64. This payload holds the same body at 0x801CF564
 * that the exact sibling lifts of this function's own body describe for their
 * payloads, and it is reached at self + 0x3E4 from here. The original calls it
 * through a plain `jal` with nothing set up in the argument registers, so it
 * takes no arguments, and this caller reads no result from $v0.
 */
void func_801CF564(void);

/* Work-sequence selector of the shared EXE image, outside this payload. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt.
 */
u32 func_8014D8D4(u8 arg0);

/* Seeded from the exact lifts of the same body at plp026 0x801D0484, plp267
 * 0x801CF75C and plp567 0x801CF560, which measured exact before this seed was
 * written. This 0x6C-byte span compared to each of those ranges over the
 * payload bytes with exactly one differing word, the first `jal` immediate at
 * +0x8, and every one of those bodies calls its own payload helper at self +
 * 0x3E4, so one body and one statement order are proven while the byte
 * difference leaves no clone. The statement order, the repeated pointer-cell
 * evaluation and the declaration style are carried over with this function's
 * own address and name.
 */
/* @source 0x801CF180
 * @behavior Work byte 3 == 0 step of this overlay: the word at 0x801D03E0 holds
 *           this address, and the byte-3 dispatcher 0x801CF13C indexes that
 *           base by work byte 3 scaled by four before its indirect call, so the
 *           body runs while that byte reads 0 and no jal in this payload names
 *           it. It first calls the payload layer-position refresh at 0x801CF564,
 *           then takes the work record published at the scratchpad pointer cell
 *           0x1F800044, reads that record's unsigned byte 8 and selects the
 *           shared work sequence through func_8014D8D4 with ((byte 8 - 1) / 2 +
 *           0x42) truncated to a byte. It then writes 0xB into work byte 0xA, the
 *           countdown the next step decrements, and 1 into work byte 3, which
 *           advances the dispatch to entry 1 of the same run at 0x801D03E4. The
 *           original re-loads the pointer cell before the byte-8 read and before
 *           each of the two stores. The subtraction leaves the division signed,
 *           so byte 8 == 0, 1 and 2 all select sequence 0x42 while byte 8 == 3
 *           gives 0x43. It takes no arguments, returns nothing, and the 0x18-byte
 *           frame only keeps $ra at 0x10 across the two calls. The roles of work
 *           byte 8, of the selected sequence and of the refresh helper's own
 *           effects are not established in this payload, so the name describes
 *           the effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshLayersThenSelectWorkSequenceFromByte8ArmCountdownAndAdvanceWorkByte3PlcharPlp46700_801CF180(
    void) {
  func_801CF564();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}
