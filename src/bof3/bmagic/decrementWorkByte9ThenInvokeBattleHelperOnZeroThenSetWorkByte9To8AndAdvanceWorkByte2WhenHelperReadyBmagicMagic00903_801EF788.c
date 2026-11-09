#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044 as g_battle_work, the
 * spelling its own lifted siblings use. The original re-materializes the cell
 * at each access group (lui $v1,0x1F80 / lw $v1,0x44($v1)) because the byte
 * stores through the loaded pointers may alias the cell, so every access stays
 * on the named cell rather than on a cached pointer local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared readiness helper of the main executable, outside this overlay's
 * image. This target's generated undefined-funcs map binds it at 0x8014DAEC,
 * and the exact sibling setWorkByte9To8AndAdvanceWorkByte2WhenHelperReady
 * BmagicMagic01703_801EF694, whose tail this function reproduces word for
 * word, declares the same unsigned byte return: the caller masks the answer
 * with andi $v0,$v0,0xFF before testing it. */
extern u8 func_8014DAEC(void);

/* Shared battle helper of the main executable, outside this overlay's image,
 * unlifted and not semantically named anywhere in the tree, so it keeps its
 * address-qualified name. The exact sibling invokeHelperThenDecrementWorkByte9
 * ThenInvokeBattleHelpersAndAdvanceWorkByte2OnZeroWhenGlobalWorkByteBIs1
 * BmagicMagic01703_801EF5FC declares the same prototype for its own 2/4 call
 * site. This call site passes only the two immediates 2 and 5 (addiu
 * $a0,$zero,0x2 and addiu $a1,$zero,0x5, the second filling the jal's delay
 * slot) and never reads $v0 afterwards, so the original constrains neither the
 * argument widths nor the return type. */
void func_800AFC98(s32 arg0, s32 arg1);

/* @source 0x801EF788
 * @behavior Step handler of this overlay's work-byte-2 handler run: the word at
 *           0x801EEC1C, index 1 of the four words at 0x801EEC18..0x801EEC27
 *           that func_801EF6B0 copies into its frame and indexes with
 *           scratchpad work-object byte 2, holds this function's address, so
 *           the body runs while work byte 2 is 1. No jal in this payload names
 *           this function, so that table word is its only incoming path. It
 *           first reads the scratchpad work-object pointer cell at 0x1F800044
 *           once, ahead of frame creation, and loads that object's unsigned
 *           countdown byte at offset 0x9 (lbu $a0,0x9($a1)), comparing it
 *           zero-extended against 0xFF (andi $v1,$a0,0xFF / beq); 0xFF is a
 *           saturated countdown that skips the decrement. Otherwise it stores
 *           the byte one lower in place (addiu $v0,$a0,-1 / sb, the decrement
 *           filling the branch delay slot) and tests the stored byte
 *           (andi $v0,$v0,0xFF / bnez), so the rest of that arm runs only when
 *           the countdown reaches zero. On zero it invokes the shared battle
 *           helper func_800AFC98 with the immediates 2 and 5. The tail runs
 *           unconditionally: it calls the shared readiness helper
 *           func_8014DAEC, masks the returned byte (andi $v0,$v0,0xFF) and
 *           branches on zero, and on a nonzero answer re-materializes the
 *           scratchpad cell and stores 8 into work byte 9 (addiu $v0,$zero,0x8
 *           materialized in the branch delay slot / sb), then re-materializes
 *           the cell once more and increments work byte 2 at byte width
 *           (lbu / addiu 1 / sb), which moves the run to index 2 at
 *           0x801EF814. The stores force each re-read because a store through
 *           the loaded pointer may republish the cell. The 0x18-byte frame only
 *           keeps $ra at 0x10 across the two calls; the function takes no
 *           arguments, returns nothing, and reads or writes no other state.
 *           Roles of work byte 9, work byte 2 and of both shared helpers are
 *           not established here, so they stay address-anchored. Statement
 *           order and operand forms follow the exact sibling
 *           dispatchPhaseCountDownStageTimerAdvanceStageTableInvokeHelperAt100
 *           AndPublishWorkWord10ToHalfword8014932ABmagicMagic05703_801EF620 for
 *           the guarded unsigned byte countdown (identical beq/andi delay-slot
 *           shape) and the exact sibling
 *           setWorkByte9To8AndAdvanceWorkByte2WhenHelperReadyBmagic
 *           Magic01703_801EF694 for the whole readiness re-arm tail. No other
 *           span in the shipped payloads carries this 0x8C-byte range, so the
 *           seed is authored here rather than cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenInvokeBattleHelperOnZeroThenSetWorkByte9To8AndAdvanceWorkByte2WhenHelperReadyBmagicMagic00903_801EF788(
    void) {
  if (g_battle_work[9] != 0xFF && --g_battle_work[9] == 0) {
    func_800AFC98(2, 5);
  }
  if (func_8014DAEC() != 0u) {
    g_battle_work[9] = 8;
    g_battle_work[2]++;
  }
}
