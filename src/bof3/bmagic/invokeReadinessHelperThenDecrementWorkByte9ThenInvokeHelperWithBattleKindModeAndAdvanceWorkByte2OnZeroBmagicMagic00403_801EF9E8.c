#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, the named cell this target's map binds.
 * The original re-materialises the cell for the countdown group and again after
 * the shared helper returns, because that call may rewrite the pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle-kind halfword of the main executable, outside this overlay's
 * image, so it keeps its address-qualified name; the sibling bmagic lifts of
 * this tree read the same halfword as the battle-kind index. */
extern u16 D_801463C0; /* @source 0x801463C0 @kind unknown */

/* Readiness poll helper of the main executable, outside this overlay's image.
 * The sibling bmagic lifts declare the same prototype; this call site issues the
 * jal and never tests the returned byte. */
u8 func_8014DAEC(void);

/* Shared battle helper of the main executable, outside this overlay's image,
 * unlifted and not semantically named anywhere in the tree, so it keeps its
 * address-qualified name. Both call sites pass a mode immediate and the
 * immediate 4 (addiu $a0 and addiu $a1) and never read $v0 afterwards, so
 * word-width arguments keep the default promotion. */
void func_800AFC98(s32 arg0, s32 arg1);

/* @source 0x801EF9E8
 * @behavior Step handler of this overlay's work-byte-2 handler run: it is entry
 *           1 of the four-word run at 0x801EEC18 (0x801EF9A0, 0x801EF9E8,
 *           0x801EFA70, 0x801EFAD4) that the dispatcher
 *           dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkByte0SetBmagicMagic00403_801EF910
 *           copies into its frame and indexes with scratchpad work-object byte
 *           2, so the body runs while work byte 2 is 1 and no jal in this
 *           payload names it. It first runs the shared readiness helper
 *           func_8014DAEC and discards the returned byte, then re-materialises
 *           the scratchpad work-object pointer cell at 0x1F800044 and
 *           decrements that object's unsigned byte 9 in place (lbu / addiu -1 /
 *           sb), testing the stored byte truncated to eight bits (andi 0xFF /
 *           bnez), so the rest of the body runs only when that countdown
 *           reaches zero. On zero it reads the shared battle-kind halfword
 *           0x801463C0 and invokes the shared battle helper func_800AFC98 with
 *           mode 2 when that kind is at least 0x85 and mode 0 otherwise, plus
 *           the immediate 4, which fills that call's delay slot. Finally it
 *           re-reads the scratchpad cell and increments work byte 2 with a
 *           byte-width add, moving the run to index 2 at 0x801EFA70. The
 *           0x18-byte frame only keeps $ra at 0x10 across the two calls; the
 *           function takes no arguments, returns nothing and reads or writes no
 *           other state. The roles of work byte 9, of the two helpers and of
 *           the mode value are not established here, so they stay
 *           address-anchored. Statement order and cell re-materialisation
 *           grouping follow the exact sibling
 *           invokeHelperThenDecrementWorkByte9ThenInvokeBattleHelpersAndAdvanceWorkByte2OnZeroWhenGlobalWorkByteBIs1BmagicMagic01703_801EF5FC,
 *           whose only differences are its outer battle-byte guard, one extra
 *           helper call on the zero path and its fixed mode immediate. The mode
 *           select keeps the call in each arm because the original materialises
 *           the zero arm in the branch delay slot and the two arm calls merge
 *           into one jal, the form the world00/area030/04 lift of 0x801D9D38
 *           also uses; a ternary or a select local folds into one shared value
 *           instead and measured two instructions short.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeReadinessHelperThenDecrementWorkByte9ThenInvokeHelperWithBattleKindModeAndAdvanceWorkByte2OnZeroBmagicMagic00403_801EF9E8(
    void) {
  func_8014DAEC();
  if (--g_battle_work[9] == 0) {
    if (D_801463C0 >= 0x85u) {
      func_800AFC98(2, 4);
    } else {
      func_800AFC98(0, 4);
    }
    g_battle_work[2]++;
  }
}
