#include "bof3/bof3.h"

void func_800A9610(u8 arg0, s32 arg1);

/* Scratchpad work-object pointer cell shared with the battle overlays. The
 * original re-materialises its address at each access (lui $vN,0x1F80 / lw
 * $vN,68($vN)) because a store through the loaded pointer may alias the cell, so
 * it stays a named cell: the constant-address accessor is folded once by CSE and
 * reused, which loads the cell through one base instead of reloading it after
 * the preceding store. Bound at 0x1F800044 by this target's map; the remaining
 * asm of this overlay spells the address as a literal, so Splat's generated
 * undefined_syms_auto.txt cannot bind it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle target byte at 0x80146384: the target selector whose 0x80/0x40
 * bits pick the party or enemy group (same cell the exact bmagic sibling
 * raiseBattleTargetFlag10WhenWorkByte9SetThenAdvanceWorkByte1BmagicMagic10003_801EED9C
 * forwards to the same helper). It lives outside this overlay and the remaining
 * asm spans func_801EED60 and func_801EEF7C still reference it, so Splat's
 * generated undefined_syms_auto.txt binds it and no target-map row is needed. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EEF1C
 * @behavior Overlay work-step handler of the record published at the scratchpad
 *           pointer cell 0x1F800044. It raises battler status flag 0x20 by
 *           passing the shared battle target byte at 0x80146384 (unsigned byte
 *           load, zero-extended) and the flag word 0x20 to func_800A9610, which
 *           ORs that flag into the selected battlers' status word. It then
 *           clears the record's byte 9 and byte 0xA and increments its byte 1 by
 *           one, so the original keeps three separate lui/lw pairs: every store
 *           through the loaded pointer may alias the cell, which forces the
 *           reload, while the last load serves both the byte-1 read and its
 *           store. The function takes no arguments, returns nothing and reads or
 *           writes no other state; the 0x18-byte frame only keeps $ra across the
 *           single call, the constant 0x20 fills that call's delay slot, and the
 *           $ra reload precedes the final byte-1 store. Statement order, operand
 *           forms and declaration style follow the exact siblings
 *           seedWorkByteBRandomNibbleClearBytes9AThenAdvanceWorkByte2BmagicMagic14303_801EEF10
 *           (call, then byte 9 and byte 0xA clears and the byte increment through
 *           the same named cell) and
 *           raiseBattleTargetFlag10WhenWorkByte9SetThenAdvanceWorkByte1BmagicMagic10003_801EED9C
 *           (the same target byte and helper call). The flag-0x20 call and the
 *           byte-1 increment with the same cell reload appear together in the
 *           unlifted func_801EEDB8 of emi/bmagic/magic066/03. No extracted payload
 *           repeats this span: the discriminating bytes (the second-store flag
 *           0x20 call followed by the byte 9/0xA clears and the byte 1
 *           increment) occur only in this target's binary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C(void) {
  func_800A9610(D_80146384, 0x20);
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 0;
  g_battle_work[1]++;
}
