#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives in the main executable, outside this
 * overlay, so it keeps its address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. It lives in the main
 * executable, outside this overlay, so it keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEE08
 * @behavior Work-step handler selected by the in-payload dispatcher at
 *           0x801EEC04, which scales the work object's byte 0x01 by four and
 *           calls the local table entry at stack offset 0x14 for index 1. It
 *           reads the scratchpad work-object pointer cell 0x1F800044 once,
 *           before the frame exists, and returns as soon as the work object's
 *           byte at offset 0xB is not 0xFF: the 0xFF immediate is materialised
 *           into a register ahead of the branch, so the test is an equality on
 *           the byte just loaded and takes no sign extension or mask. Only on
 *           that equality does it load the shared battle byte at 0x80146384
 *           unsigned and pass it to the shared helper func_800A9820, raise bit
 *           0x4 of the shared battle global halfword at 0x801462E8 (unsigned
 *           halfword load, ori, halfword store that re-materialises its address
 *           into $at) and then invoke the shared helper func_801E5988. It takes
 *           no arguments, returns nothing and writes no other state; the
 *           0x18-byte frame only keeps $ra at 0x10 across the two calls.
 *           Statement order, operand forms, offset-local spelling and
 *           declaration style follow the byte-identical lifted sibling
 *           forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBIsFF
 *           BmagicMagic10703_801EED40, whose 0x60-byte body was measured equal
 *           to this one across all 96 bytes; the same 0x60 bytes also appear
 *           unchanged in the six unlifted overlay copies
 *           emi/bmagic/magic066/03@0x801EEE5C, magic078/03@0x801EEE84,
 *           magic086/03@0x801EF168, magic108/03@0x801EED1C,
 *           magic120/03@0x801EEFB0 and magic122/03@0x801EEDFC. The clone
 *           adapter declined this span because the seed reaches the pointer
 *           cell through the SPAD_PTR_SLOT macro, which it reads as an
 *           unresolved external, so the body was seeded from the sibling by
 *           hand.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBIsFFBmagicMagic12103_801EEE08(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0xFF) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
