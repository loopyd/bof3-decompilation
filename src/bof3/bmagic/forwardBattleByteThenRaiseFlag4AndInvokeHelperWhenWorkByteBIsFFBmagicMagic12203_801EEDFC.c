#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Scratchpad work-object pointer cell. The original re-materialises its address
 * in the load itself (lui $v0,0x1F80 / lw $v0,0x44($v0)), so it stays a named
 * cell bound at 0x1F800044 by this target's map, the same row the exact siblings
 * incrementWorkByte9AndAdvanceWorkByte2At78BmagicMagic12203_801EF24C and
 * decrementWorkByteBIncrementByte9RaiseFlag10At18ThenAdvanceWorkByte2
 * BmagicMagic12203_801EF1C4 use. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle target byte at 0x80146384: the zero-extended battle target byte
 * that func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. It lives outside this overlay,
 * so it keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEDFC
 * @behavior Work-step handler of this overlay: it reads the scratchpad
 *           work-object pointer cell at 0x1F800044 once, before the frame
 *           exists, and returns as soon as the work-object byte at offset 0xB is
 *           not 0xFF. The 0xFF immediate is materialised into a register ahead
 *           of the branch, so the test is an equality on the byte just loaded
 *           and takes no sign extension or mask. Only on that equality does it
 *           load the shared battle byte at 0x80146384 unsigned and pass it to
 *           the shared helper func_800A9820, raise bit 0x4 of the shared battle
 *           global halfword at 0x801462E8 (unsigned halfword load, ori, halfword
 *           store that re-materialises its address into $at) and then invoke the
 *           shared helper func_801E5988. It takes no arguments, returns nothing
 *           and writes no other state; the 0x18-byte frame only keeps the return
 *           address at 0x10 across the two calls. No in-image jal or pointer
 *           word names this address, so it is entered from outside the payload.
 *           Statement order, operand forms and declaration style follow the
 *           byte-identical exact sibling
 *           forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBIsFF
 *           BmagicMagic10703_801EED40, whose 0x60-byte body measured equal to
 *           this span across all 96 bytes; the clone adapter declined this span
 *           because that representative reaches the pointer cell through the
 *           SPAD_PTR_SLOT macro, which it reads as an unresolved external, so
 *           the body was seeded from the sibling by hand. The same 0x60 bytes
 *           also stand unlifted in emi/bmagic/magic066/03@0x801EEE5C,
 *           magic078/03@0x801EEE84, magic086/03@0x801EF168,
 *           magic108/03@0x801EED1C, magic120/03@0x801EEFB0 and
 *           magic121/03@0x801EEE08.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBIsFFBmagicMagic12203_801EEDFC(void) {
  if (g_battle_work[0xB] == 0xFF) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
