#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, the same named cell this target's exact
 * lifts seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2ThenInvokeSharedService
 * BmagicMagic00800_801F09D0 and
 * invokeReadinessHelperThenDecrementWorkByte9ThenInvokeBattleHelpersAndAdvance
 * WorkByte2OnZeroBmagicMagic00800_801F1384 declare. The named symbol lets the
 * assembler fold the constant address into each load (lui $v1,0x1F80 / lw
 * $v1,0x44($v1)), and every scratchpad access in the original re-reads the
 * cell, so this body indexes the cell directly instead of a saved pointer. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared readiness helper of the main executable at 0x8014DAEC, outside this
 * overlay's image; the exact siblings
 * invokeReadinessHelperThenDecrementWorkByte9ThenInvokeBattleHelpersAndAdvance
 * WorkByte2OnZeroBmagicMagic00800_801F1384 and
 * setGlobalWorkByteBToFFAtWorkByte2State5ThenAdvanceWorkByte2WhenHelperReady
 * BmagicMagic00800_801F1400 declare the same prototype. This body calls it on
 * both arms and tests the returned byte truncated to eight bits on the second
 * one. */
u8 func_8014DAEC(void);

/* Shared helper of the main executable at 0x800AFC98, outside this overlay's
 * image: invoked here with the constant pair (2, 4) on both arms and its result
 * discarded, so the original constrains neither argument width and word-width
 * arguments keep the default promotion. */
void func_800AFC98(s32 arg0, s32 arg1);

/* Shared main-executable service func_8014D290 is declared per caller in this
 * family, not in a shared header; this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x801F0A20
 * @behavior Entry 1 of the four-entry work-state table at 0x801EEC84
 *           (0x801F09D0, 0x801F0A20, 0x801F0AC8, 0x801F0B44), which the
 *           dispatcher at 0x801F0964 copies into its frame and selects with
 *           scratchpad work-object byte 2. No jal in this payload names this
 *           function, so that table word is its only incoming path. Per frame
 *           it re-reads the work-object pointer cell at 0x1F800044 and
 *           decrements that object's unsigned byte 9 in place (lbu / addiu -1
 *           / sb), testing the stored byte truncated to eight bits (andi 0xFF /
 *           bnez). When that countdown reaches zero it invokes the shared
 *           readiness helper func_8014DAEC, discards the returned byte, and
 *           then invokes the shared helper func_800AFC98 with the constant pair
 *           (2, 4). While the countdown is still non-zero it invokes
 *           func_8014DAEC again and tests the returned byte truncated to eight
 *           bits: a zero answer leaves the rest of the body, so only the
 *           service call at the tail runs, while a non-zero answer invokes
 *           func_800AFC98(2, 4), re-reads the pointer cell and stores 0 into
 *           work byte 9. Both arms that reach the shared tail re-read the
 *           pointer cell once more and increment work byte 2 with a byte-width
 *           add, moving dispatch to entry 2 at 0x801F0AC8, and then invoke the
 *           shared main-executable service at 0x8014D290 unconditionally. The
 *           0x18-byte frame keeps only $ra at 0x10 across the three calls; the
 *           function takes no arguments, returns nothing and reads or writes no
 *           other state. Statement order, cell re-materialization grouping and
 *           declaration style follow the exact siblings of this target:
 *           0x801F1384 for the byte-width countdown test that compares the
 *           stored byte against zero and the trailing byte-2 increment through
 *           a fresh cell read, and 0x801F1090 for the (2, 4) helper pair with
 *           its word-width constant arguments. The roles of work byte 9, of the
 *           three shared helpers and of the two constants are not established
 *           here, so they stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenInvokeHelperPairWhenZeroOrReadinessHelperReadyClearWorkByte9AndAdvanceWorkByte2BmagicMagic00800_801F0A20(
    void) {
  if (--g_battle_work[9] == 0) {
    func_8014DAEC();
    func_800AFC98(2, 4);
  } else if (func_8014DAEC() != 0) {
    func_800AFC98(2, 4);
    g_battle_work[9] = 0;
  } else {
    goto service;
  }
  g_battle_work[2]++;
service:
  func_8014D290();
}
