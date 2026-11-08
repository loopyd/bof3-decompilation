#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * with its own `lui` before each of the three reads, so the source keeps the
 * named symbol form rather than SPAD_PTR_SLOT, whose bare constant address is
 * hoisted once into a base register. The exact sibling
 * dropFrontEndRecordSelectionAndStepBackDispatchStatePlcharPlp02600_801D00B4
 * binds the same cell the same way.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Two-layer position refresh of this payload: its body queries the shared
 * route-parameterised position routine func_80154F78 and the screen-space
 * routine func_80154F28 for the work record's coord_x_34/coord_y_38 pair over
 * the two layer bases at 0x80181BAC/0x80181BB0 and 0x80181BBC/0x80181BC0, and
 * clears work byte 0x2B when a layer sits in front. Its Splat row is a `c` row
 * of this payload, lifted as
 * claimWorkByte2BThenRefreshTwoLayerPositionsAndClearWhenLayerInFrontPlcharPlp02600_801D0868;
 * the original calls it through a plain `jal` with nothing set up in the
 * argument registers and reads no result from $v0. The exact PLP016 sibling
 * calls the same routine at its own overlay-local address 0x801D0444. */
void claimWorkByte2BThenRefreshTwoLayerPositionsAndClearWhenLayerInFrontPlcharPlp02600_801D0868(
    void);

/* Work-sequence selector of the shared EXE image, outside this payload. The ui
 * and bmagic lifts (for example src/bof3/ui/selectWorkSequenceForRoute7.c)
 * declare the same prototype and pass a sequence id; Splat binds the address
 * through this target's generated undefined_funcs_auto.txt, and the remaining
 * `asm` rows of this payload reference it too. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801D0484
 * @behavior Work byte 3 == 0 step of this overlay: entry 0 of the work-byte-3
 *           handler run based at 0x801D0AA8, the word at 0x801D0AA8 inside this
 *           payload's thirty-word code-pointer run at 0x801D0A54, so it is the
 *           handler the byte-3 dispatcher
 *           dispatchStateByte3Table0aa8PlcharPlp02600_801D0440 selects while the
 *           work record's dispatch byte 3 reads zero, and it advances that byte
 *           to 1, the step-1 entry
 *           selectWorkSequenceFromByte8AndAdvanceWorkByte3WhenHelperReadyElseTickCountdownByteAThenSeedFrontEndRowAndQueueCue2CPlus100Then10bPlcharPlp02600_801D04F0.
 *           It first refreshes this payload's two layers through the
 *           position-query routine at 0x801D0868, then
 *           takes the work record published at the scratchpad pointer cell
 *           0x1F800044, reads that record's unsigned byte 8 and selects the
 *           shared work sequence through func_8014D8D4 with ((byte 8 - 1) / 2 +
 *           0x42) truncated to a byte. The subtraction leaves the division
 *           signed, so byte 8 == 0 still selects sequence 0x42. It then writes
 *           0xB into work byte 0x0A, the counter the step-1 handler at
 *           0x801D04F0 decrements, and 1 into work byte 3. Each of the three
 *           pointer-cell reads re-materialises 0x1F800044 with its own lui: the
 *           second follows the call and the third follows a store through the
 *           record, which may alias the cell. The 0x18-byte frame only keeps
 *           $ra at 0x10 across the two calls. It takes no arguments, returns
 *           nothing, and no jal targets it: the word 0x801D0AA8 is its only
 *           in-image reference, so the overlay loader that steps the run is what
 *           calls it and its prologue and return path alone place the boundary.
 *           Its 108-byte body matches the exact PLP016 lift
 *           refreshLayersThenSelectWorkSequenceFromByte8HalfArmCountdownAndAdvanceWorkByte3PlcharPlp01600_801D0060
 *           byte for byte except for the first jal word, whose target is
 *           0x801D0444 there and 0x801D0868 here; a cmp of the two payload spans
 *           reports 106 of 108 bytes equal with bytes 8 and 9 differing, so that
 *           sibling is shape evidence and not a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshLayersThenSelectWorkSequenceFromByte8ArmCountdownAndAdvanceWorkByte3PlcharPlp02600_801D0484(
    void) {
  claimWorkByte2BThenRefreshTwoLayerPositionsAndClearWhenLayerInFrontPlcharPlp02600_801D0868();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}
