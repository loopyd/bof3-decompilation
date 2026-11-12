#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, the same named cell this target's exact
 * lifts decrementMotionCountdownAndAdvanceMotionPairThenSeedWorkByte9FromHelper
 * AFF98AndAdvanceWorkByte2BmagicMagic00800_801F037C and
 * invokeHelperAt101SetByte9ToFAndAdvanceWorkByte2WhenHelperReadyBmagic
 * Magic00800_801F047C declare. The named symbol lets the assembler fold the
 * constant address into each load (lui $v1,0x1F80 / lw $v1,0x44($v1)), and the
 * original re-materializes the cell for the byte 9 countdown group and again
 * for the trailing byte 2 update because a store through the pointer may alias
 * the cell object. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte in main RAM, outside this overlay's image: the byte this
 * body forwards to the helper at 0x800A9820 when the countdown reaches zero.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt row, so no target-map row is added here. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Shared readiness helper of the main executable at 0x8014DAEC, outside this
 * overlay's image. The exact siblings invokeHelperAt101SetByte9ToFAndAdvance
 * WorkByte2WhenHelperReadyBmagicMagic00800_801F047C and
 * setGlobalWorkByteBToFFAtWorkByte2State5ThenAdvanceWorkByte2WhenHelperReady
 * BmagicMagic00800_801F1400 declare the same prototype and test the returned
 * byte. This call site issues the jal and never reads $v0, so the answer is
 * discarded. */
u8 func_8014DAEC(void);

/* Shared helper of the main executable at 0x800A9820, outside this overlay's
 * image: its one argument is forwarded from the shared battle byte 0x80146384,
 * the same zero-extended byte argument the exact sibling
 * invokeReadinessHelperThenDecrementWorkByte9ThenInvokeBattleHelpersAndAdvance
 * WorkByte2OnZeroBmagicMagic00800_801F1804 declares. Its effect is not
 * established, so the raw address name stays. */
void func_800A9820(u8 arg0);

/* Shared helper of the main executable at 0x800AFC98, outside this overlay's
 * image: invoked here with the constant pair (2, 4) and its result discarded,
 * so the original constrains neither argument width and word-width arguments
 * keep the default promotion. Its effect and the meaning of the two immediates
 * are not established, so the raw address name stays and no map row is needed. */
void func_800AFC98(s32 arg0, s32 arg1);

/* @source 0x801F03FC
 * @behavior Entry 2 (index 2) of the six-entry handler table at 0x801EEC58
 *           (0x801F0330, 0x801F037C, 0x801F03FC, 0x801F047C, 0x801F04DC,
 *           0x801F0548), which 0x801F0294 copies into its own 0x30-byte frame
 *           and dispatches on scratchpad work-object byte 0x2, the state
 *           0x801F037C advances from 1, so this body runs on the frame where
 *           that byte holds 2. Per frame it calls the shared readiness helper
 *           func_8014DAEC and discards the returned byte, then re-materializes
 *           the work-object pointer cell at 0x1F800044 and decrements that
 *           object's unsigned byte 9 in place (lbu / addiu -1 / sb), testing
 *           the stored byte truncated to eight bits (andi 0xFF / bnez), so the
 *           rest of the body runs only when that countdown reaches zero. On
 *           zero it forwards the shared battle byte 0x80146384 to the shared
 *           helper func_800A9820, then invokes the shared helper func_800AFC98
 *           with the constant pair (2, 4), and finally re-reads the pointer
 *           cell and increments work byte 0x2 with a byte-width add (lbu /
 *           addiu 1 / sb), handing dispatch to table entry 3 (0x801F047C). The
 *           0x18-byte frame only keeps $ra at 0x10 across the calls; the
 *           function takes no arguments, returns nothing and reads or writes no
 *           other state.
 *           One cmp of the payload spans proved this function is not
 *           byte-identical to any other span in the payload, so the nearest
 *           sibling invokeReadinessHelperThenDecrementWorkByte9ThenInvokeBattle
 *           HelpersAndAdvanceWorkByte2OnZeroBmagicMagic00800_801F1804 (0x7C
 *           bytes at 0x2C04) is shape evidence only: its first 0x30 bytes equal
 *           this span's, and its countdown group, readiness call, trailing byte
 *           2 increment, frame and epilogue are the same straight-line code.
 *           This body differs from it in the zero arm only - it forwards the
 *           battle byte first and takes (2, 4) where that sibling takes (2, 0) -
 *           which is also why the original leaves nop in the bnez delay slot
 *           here while that sibling hoists the first argument's 2 into its own.
 *           Statement order, operand forms, declaration style and cell bindings
 *           are transcribed from that sibling; the order of the two calls in
 *           the zero arm and the second helper argument are written from this
 *           span's own bytes. The roles of work byte 9, of the three shared
 *           helpers and of both immediates are not established here, so they
 *           stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeReadinessHelperThenDecrementWorkByte9AndOnZeroForwardBattleByteThenInvokeHelperPairAndAdvanceWorkByte2BmagicMagic00800_801F03FC(void) {
  func_8014DAEC();
  if (--g_battle_work[9] == 0) {
    func_800A9820(D_80146384);
    func_800AFC98(2, 4);
    g_battle_work[2]++;
  }
}
