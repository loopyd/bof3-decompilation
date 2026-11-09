#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it before the
 * record-byte read and again before each of the two stores, so the source keeps
 * the named symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Layer-position refresh of this payload, lifted as
 * claimWorkByte2BThenRefreshTwoLayerPositionsAndClearWhenLayerInFrontPlcharPlp67800_801CF348;
 * its bytes at 0x801CF348 claim work byte 0x2B, draw the work reference
 * positions and clear the claim again when a probe reads above 64, which is the
 * body the exact sibling lifts of this function's own body describe for their
 * payloads. This caller reaches it through the plain `jal` at self + 8 with
 * nothing set up in the argument registers, so it takes no arguments, and this
 * caller reads no result from $v0.
 */
void claimWorkByte2BThenRefreshTwoLayerPositionsAndClearWhenLayerInFrontPlcharPlp67800_801CF348(
    void);

/* Work-sequence selector of the shared EXE image, outside this payload. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CEF64
 * @behavior Work byte 3 == 0 step of this overlay: the word at 0x801D058C is
 *           the table base the byte-3 dispatcher 0x801CEF20 indexes by work
 *           byte 3, and its first entry is this body, so that dispatcher
 *           reaches it with index zero and no jal in this payload names it. It
 *           first calls the payload layer-position refresh at 0x801CF348, then
 *           takes the work record published at the scratchpad pointer cell
 *           0x1F800044, reads that record unsigned byte 8 and selects the shared
 *           work sequence through func_8014D8D4 with ((byte 8 - 1) / 2 + 0x42)
 *           truncated to a byte. It then writes 0xB into work byte 0xA, the
 *           countdown the byte-3 entry 1 handler at 0x801CEFD0 decrements, and 1
 *           into work byte 3, which moves the dispatch to that same neighbour.
 *           The original re-loads the pointer cell before the byte-8 read and
 *           before each of the two stores. The subtraction leaves the division
 *           signed, so byte 8 == 0, 1 and 2 all select sequence 0x42 while byte
 *           8 == 3 gives 0x43. It takes no arguments, returns nothing, and the
 *           0x18-byte frame only keeps $ra at 0x10 across the two calls. The
 *           0x6C-byte body differed from the already exact lifts of the same
 *           body at plp467 0x801CF180 and plp567 0x801CF560 in exactly its
 *           first jal word when this seed was written, so those siblings are
 *           shape evidence, not clones, and their statement order, operand forms
 *           and declaration style are carried over with this function literals.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshLayersThenSelectWorkSequenceFromByte8ArmCountdownAndAdvanceWorkByte3PlcharPlp67800_801CEF64(
    void) {
  claimWorkByte2BThenRefreshTwoLayerPositionsAndClearWhenLayerInFrontPlcharPlp67800_801CF348();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}
