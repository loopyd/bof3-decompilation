#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The compiled original re-materializes the cell address for each of its three
 * accesses (lui $v0,0x1F80 / lw $v0,0x44($v0) twice, then lui $v1 / lw
 * $v1,0x44($v1)), so the cell is reached through this target-map symbol and each
 * access keeps its own pointer local, because a store through the loaded record
 * pointer may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-executable resource request helper at 0x8014D6B8, outside this
 * overlay image. This body passes it the resource id 3 and discards the answer;
 * the boss and bmagic lifts of this family pass it a numeric resource id the
 * same way. Splat binds the address through this target generated
 * undefined_funcs_auto.txt row, so no target-map row is added here. */
extern void func_8014D6B8(u32 id); /* @source 0x8014D6B8 @kind function */

/* Shared main-executable update helpers at 0x8014D978 and 0x8014D4E0, outside
 * this overlay image. This body calls both with no arguments and discards their
 * results, so their roles stay address-anchored. */
extern void func_8014D978(void); /* @source 0x8014D978 @kind function */
extern void func_8014D4E0(void); /* @source 0x8014D4E0 @kind function */

/* @source 0x801F001C
 * @behavior Entry index 0 of the three-word work-byte-2 handler table based at
 *           0x801EEC10 (0x801F001C, 0x801F007C, 0x801F00D8), which the
 *           dispatcher at 0x801EFFBC copies into its own frame and indexes with
 *           the unsigned work byte 2 of the scratchpad work object reached
 *           through 0x1F800044; no jal in this payload names this span, so that
 *           table word proves the function-entry boundary at 0x801F001C. It
 *           clears work byte 0xB, re-reads the pointer cell and clears work byte
 *           9, requests resource 3 through the shared main-executable helper at
 *           0x8014D6B8, runs the shared updates 0x8014D978 and 0x8014D4E0 in
 *           that order, then re-reads the cell once more and increments work
 *           byte 2 modulo 256, which advances the byte-2 dispatcher to entry 1.
 *           The 0x18-byte non-leaf frame keeps only $ra at 0x10 across the three
 *           calls and the first cell read precedes frame creation. It takes no
 *           arguments, returns nothing through the jr $ra at 0x801F0074 and
 *           writes no state outside the work object. One payload cmp of the
 *           96-byte range proved this span differs from the exact lifted sibling
 *           at 0x800C1F44 of emi/boss/boss036/16 in exactly one word, offset
 *           0x1C, the resource id (0x03000424 here against 0x01000424 there), so
 *           the statement order, operand forms, pointer-local spelling and
 *           declaration style are transcribed from that sibling, which records
 *           this span as its nearest twin, and this body writes its own resource
 *           id 3 from its own bytes. The close same-target sibling at 0x801F03EC
 *           of emi/bmagic/magic067/03 carries the same clears and resource
 *           request but neither shared update call (0x10 bytes shorter), so it
 *           supplies shape only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateBytesRequestInvoke(
    void) {
  u8 *work;
  u8 *slot;
  u8 *step;

  work = g_battle_work;
  work[0xB] = 0;
  slot = g_battle_work;
  slot[0x9] = 0;
  func_8014D6B8(3);
  func_8014D978();
  func_8014D4E0();
  step = g_battle_work;
  step[0x2]++;
}
