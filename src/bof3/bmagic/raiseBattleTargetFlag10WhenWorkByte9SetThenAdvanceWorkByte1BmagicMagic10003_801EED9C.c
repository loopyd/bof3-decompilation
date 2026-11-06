#include "bof3/bof3.h"

void func_800A9610(u8 arg0, s32 arg1);

/*
 * Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The cell is read once per access, so the original re-materializes its address
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) before each of the two accesses; the cell
 * is therefore reached through this target-map symbol rather than through a
 * constant address. Splat's generated undefined_syms_auto.txt cannot bind it
 * (the remaining asm of this overlay spells 0x1F800044 as a literal), so the
 * target map owns the address.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * Shared battle target byte at 0x80146384: the target selector that the battle
 * helpers consume (its 0x80/0x40 bits pick the party or enemy group). It lives
 * outside this overlay, so it keeps its address-qualified name.
 */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EED9C
 * @behavior Re-reads the scratchpad work-object pointer cell at 0x1F800044 and
 *           returns immediately when that work object's byte at offset 0x9 is
 *           zero. Otherwise it raises the battler status flag 0x10 by passing
 *           the shared battle target byte at 0x80146384 (unsigned byte load,
 *           zero-extended) and the flag word 0x10 to func_800A9610, which ORs
 *           0x10 into the selected battlers' status word, and then re-reads the
 *           work-object pointer cell and increments that work object's byte at
 *           offset 0x1 with a byte-width read-modify-write (lbu, addiu
 *           $v0,$v0,1, sb). Nothing is returned, no argument is read and the
 *           0x18-byte frame only keeps $ra across the single call. The cell
 *           address is re-materialised for each access because the original
 *           shows a lui/lw pair per access, and the flag call's argument load
 *           fills that call's delay slot. The nearest sibling is the unlifted
 *           func_801EEDB8 of emi/bmagic/magic066/03, which countdowns the same
 *           work byte and calls the same helper with flag 0x20; it is shape
 *           evidence only, because no extracted payload contains a
 *           byte-identical copy of these 92 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseBattleTargetFlag10WhenWorkByte9SetThenAdvanceWorkByte1BmagicMagic10003_801EED9C(void) {
  if (g_battle_work[9] != 0) {
    func_800A9610(D_80146384, 0x10);
    g_battle_work[1] += 1;
  }
}
