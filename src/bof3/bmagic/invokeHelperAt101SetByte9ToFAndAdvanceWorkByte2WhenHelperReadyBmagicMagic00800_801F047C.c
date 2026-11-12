#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the exact siblings
 * advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C and
 * decrementMotionCountdownAndAdvanceMotionPairThenSeedWorkByte9FromHelperAFF98
 * AndAdvanceWorkByte2BmagicMagic00800_801F037C use: the original re-reads the
 * cell before each of this function's two store groups, and the named symbol
 * lets the assembler fold the constant address into the load
 * (lui $v1,0x1F80 / lw $v1,68($v1)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared readiness helper, outside this overlay's image. It reports a readiness
 * byte, which the exact main-executable lifts (clearWorkFlagsWhenReady,
 * stepWorkSequenceAndClearByte12BOnNonzero) read as a non-zero ready flag, and
 * the exact sibling advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C
 * declares the same u8 return for the same andi 0xff test of the same result.
 * Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is needed. */
u8 func_8014DAEC(void);

/* Helper of the main executable at 0x8015D71C. The bmagic lifts pass it a
 * constant selector (0x100 in seedMotionTintAndChannelFieldsThenAdvanceWorkByte2
 * BmagicMagic00800_801F05FC, 0x101 here, 0x103 in decrementWorkByte9ThenQueueSlot
 * ClearByte1PublishWorkPointerIncrementWorkByteBInvokeHelperAt103RearmByte9To20
 * AndAdvanceWorkByte2WhenZeroBmagicMagic15103_801EFC68) and declare the same
 * one-argument prototype. Splat binds the address through this target's
 * generated undefined_funcs_auto.txt, so no target-map row is needed. */
void func_8015D71C(s32 arg0);

/* @source 0x801F047C
 * @behavior Entry 3 (index 3) of the six-entry handler table at 0x801EEC58
 * (0x801F0330, 0x801F037C, 0x801F03FC, 0x801F047C, 0x801F04DC, 0x801F0548),
 * which 0x801F0294 copies into its frame and dispatches on scratchpad work
 * object byte 0x2, so this body runs on the frame where that byte holds 3. It
 * calls the shared readiness helper func_8014DAEC and tests its returned byte;
 * while the helper reports not-ready the handler does nothing and byte 0x2 stays
 * 3. Once it reports ready the handler invokes the shared helper at 0x8015D71C
 * with the selector 0x101, stores 0x0F into the work object byte 0x9 (the
 * countdown 0x801F0330 seeds with 0x1E), and then re-reads the pointer cell at
 * 0x1F800044 and increments work byte 0x2 with a byte-width read-modify-write
 * (lbu / addiu 1 / sb), handing dispatch to table entry 4 (0x801F04DC). The
 * original materialises the cell address separately for the byte 0x9 store and
 * for the byte 0x2 update, which the volatile cell reproduces. The 0x18-byte
 * frame only keeps $ra at 0x10 across the helper call; the function returns
 * nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperAt101SetByte9ToFAndAdvanceWorkByte2WhenHelperReadyBmagicMagic00800_801F047C(void) {
  if (func_8014DAEC() != 0u) {
    func_8015D71C(0x101);
    g_battle_work[9] = 0xF;
    g_battle_work[2]++;
  }
}
