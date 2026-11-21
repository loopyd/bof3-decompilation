#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. This target's map binds it at 0x1F800044 as g_battle_work, the
 * same binding its exact siblings decrementWorkByte9ThenSetGlobalWorkByteBTo
 * FFAndInvokeHelperWhenZeroBmagicMagic07803_801EF2CC and
 * incrementWorkByte9Until8ThenInvokeHelperAt101AndAdvanceWorkByte2Bmagic
 * Magic07803_801F00EC use. The original loads the cell once (lui $v0,0x1F80 /
 * lw $v0,0x44($v0)) before the 0x18-byte frame is created and reaches the
 * guarded byte 0xB access through that one pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives in the main executable, outside this
 * overlay, so it keeps its address-qualified name; Splat's generated
 * undefined_syms_auto.txt already binds the address for the asm spans. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. It lives in the main
 * executable, outside this overlay, so it keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEE84
 * @behavior Entry index 1 of the two-entry work-byte-1 handler run that the
 *           per-frame driver at 0x801EEC04 builds on its own stack: that
 *           driver writes this span to 0x1C($sp) with a lui/addiu pair, so the
 *           span has no jal immediate and no payload word names its address.
 *           It loads the scratchpad work-object pointer cell at 0x1F800044
 *           once, before the 0x18-byte frame exists, and compares the work
 *           object's unsigned byte at offset 0xB against 0xFF, which is
 *           materialised into a register ahead of the branch instead of being
 *           tested against zero or a mask. Only on that equality does it load
 *           the shared battle byte at 0x80146384 unsigned and pass it to the
 *           shared helper func_800A9820, raise bit 0x4 of the shared battle
 *           global halfword at 0x801462E8 (unsigned halfword load, ori,
 *           halfword store that re-materialises the address into $at) and then
 *           invoke the shared helper func_801E5988. It takes no arguments,
 *           returns nothing and writes no other state; the 0x18-byte frame only
 *           keeps $ra at 0x10 across the two calls.
 *
 *           Statement order, operand forms, offset-local spelling and
 *           declaration style follow the exact sibling
 *           forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBIsFF
 *           BmagicMagic12103_801EEE08, whose own 0x60-byte body a payload
 *           compare measured equal to this span across all 96 bytes; the same
 *           0x60 bytes also stand unchanged at magic066/03@0x801EEE5C,
 *           magic086/03@0x801EF168, magic107/03@0x801EED40,
 *           magic108/03@0x801EED1C, magic120/03@0x801EEFB0 and
 *           magic122/03@0x801EEDFC. The clone adapter declined the span
 *           because the sibling body reaches that cell through the
 *           SPAD_PTR_SLOT macro, so this body was transcribed by hand onto
 *           the target-local g_battle_work binding.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBIsFFBmagicMagic07803_801EEE84(void) {
  u8 *work = g_battle_work;

  if (work[0xB] == 0xFF) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
