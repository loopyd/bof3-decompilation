#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated undefined_syms_auto
 * file, and every access here is byte-addressed, so the local declaration keeps
 * the byte view only. The exact siblings of this target declare it the same
 * way. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named fixed address, the
 * form this target's exact siblings use, so each access group re-materializes
 * the cell with its own lui plus offset load; the original re-reads it after
 * every call because a store through the pointer may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared readiness helper of the main executable, outside this overlay's image;
 * the exact sibling setWorkByte9To8AndAdvanceWorkByte2WhenHelperReadyBmagic
 * Magic01703_801EF694 declares the same prototype and tests the returned byte.
 * This call site discards the answer, which is why the original issues the jal
 * without any test on $v0. Splat binds the address through this target's
 * generated undefined_funcs_auto file, so no target-map row is needed. */
extern u8 func_8014DAEC(void);

/* Shared battle helper of the main executable, outside this overlay's image; it
 * takes the battle target byte zero-extended in one register argument. The
 * exact sibling forwardBattleByteThenRaiseFlag4AndInvokeHelperBmagic
 * Magic01000_801EEF58 declares the same prototype. */
void func_800A9820(u8 arg0);

/* Shared battle helper of the main executable, outside this overlay's image,
 * unlifted and not semantically named anywhere in the tree, so it keeps its
 * address-qualified name. This call site passes only the two immediates 2 and 4
 * (addiu $a0,$zero,2 and addiu $a1,$zero,4) and never reads $v0 afterwards, so
 * the original constrains neither the argument widths nor the return type;
 * word-width arguments keep the default promotion and the discarded result
 * needs no declaration. */
void func_800AFC98(s32 arg0, s32 arg1);

/* @source 0x801EF5FC
 * @behavior Step handler of this overlay's work-byte-2 handler run: the word at
 *           0x801EEC20 holds this function's address, index 4 of the ten words
 *           the run dispatcher func_801EF058 copies from 0x801EEC10 into its
 *           frame and indexes with scratchpad work-object byte 2, so the body
 *           runs while work byte 2 is 4. That dispatcher is the single entry of
 *           the run selected by
 *           dispatchWorkByte1HandlerBmagicMagic01703_801EF014, and no jal in
 *           this payload names this function, so the table word is its only
 *           incoming path. It first reads the shared battle work record pointer
 *           at 0x801EB4E0 once, ahead of frame creation, and compares that
 *           record's byte 0xB against one (lbu $v1 / addiu $v0,$zero,1 / bne,
 *           empty delay slot); anything but one branches straight to the
 *           epilogue. On one it runs the shared readiness helper func_8014DAEC
 *           and discards the returned byte, then re-materializes the scratchpad
 *           work-object pointer cell at 0x1F800044 and decrements that object's
 *           unsigned byte 9 in place (lbu / addiu -1 / sb), testing the stored
 *           byte truncated to eight bits (andi $v0,$v0,0xFF / bnez), so the
 *           rest of the body runs only when that countdown reaches zero. On
 *           zero it reads the shared battle byte 0x80146384 with an unsigned
 *           byte load, passes it zero-extended to the shared battle helper
 *           func_800A9820, then invokes the shared battle helper func_800AFC98
 *           with the immediates 2 and 4, the second of which fills that call's
 *           delay slot. Finally it re-reads the scratchpad cell and increments
 *           work byte 2 with a byte-width add, moving the run to index 5 at
 *           0x801EF694. The 0x18-byte frame only keeps $ra at 0x10 across the
 *           three calls; the function takes no arguments, returns nothing, and
 *           reads or writes no other state. Roles of work byte 9 and of the
 *           three shared helpers are not established here, so they stay
 *           address-anchored. Statement order and cell re-materialization
 *           grouping follow the exact siblings of the same run in this target:
 *           setWorkByte9To8AndAdvanceWorkByte2WhenHelperReadyBmagic
 *           Magic01703_801EF694 for the frame, the readiness call and the
 *           trailing byte-2 increment, and
 *           decrementWorkByte9ThenInvokeActingEnemyScriptSetFlag20Mode1SeedByte2
 *           BAndWorkColorTripletToC0SetWorkByte9To8AndAdvanceWorkByte2Bmagic
 *           Magic01703_801EF6EC for the in-place byte-9 countdown and its
 *           truncated test.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperThenDecrementWorkByte9ThenInvokeBattleHelpersAndAdvanceWorkByte2OnZeroWhenGlobalWorkByteBIs1BmagicMagic01703_801EF5FC(void) {
  if (D_801EB4E0[0xB] == 1) {
    func_8014DAEC();
    if (--D_1F800044[9] == 0) {
      func_800A9820(D_80146384);
      func_800AFC98(2, 4);
      D_1F800044[2]++;
    }
  }
}
