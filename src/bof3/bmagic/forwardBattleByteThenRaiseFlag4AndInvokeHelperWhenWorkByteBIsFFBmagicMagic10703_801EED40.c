#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind the
 * same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EED40
 * @behavior Overlay work-step handler for the work object published at the
 *           scratchpad pointer cell 0x1F800044. It reads the cell once, before
 *           the frame exists, and returns as soon as the work object's byte at
 *           offset 0xB is not 0xFF: the 0xFF immediate is materialised into a
 *           register ahead of the branch, so the test is an equality on the
 *           byte just loaded and takes no sign extension or mask. Only on that
 *           equality does it load the shared battle byte at 0x80146384
 *           unsigned and pass it to the shared helper func_800A9820, raise bit
 *           0x4 of the shared battle global halfword at 0x801462E8 (unsigned
 *           halfword load, ori, halfword store that re-materialises its address
 *           into $at) and then invoke the shared helper func_801E5988. It takes
 *           no arguments, returns nothing and writes no other state; the
 *           0x18-byte frame only keeps $ra at 0x10 across the two calls.
 *           Statement order, operand forms and declaration style follow the
 *           exact siblings
 *           forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic10603_801EEE80
 *           (the same cell-before-frame pointer load through SPAD_PTR_SLOT, the
 *           same byte-0xB guard on that object and the same forward, raise and
 *           invoke tail) and
 *           forwardBattleByteThenRaiseFlag4AndInvokeHelperBmagicMagic01000_801EEF58
 *           (the same tail without a guard); this body's own 0xFF discriminator
 *           comes from its original bytes. The 96 bytes are byte-identical to
 *           seven other unlifted overlay copies (emi/bmagic/magic066/03
 *           @0x801EEE5C, magic078/03@0x801EEE84, magic086/03@0x801EF168,
 *           magic108/03@0x801EED1C, magic120/03@0x801EEFB0,
 *           magic121/03@0x801EEE08 and magic122/03@0x801EEDFC); none of those
 *           copies is lifted, so this source was seeded from the original bytes
 *           instead of cloned. The in-payload dispatcher at 0x801EEC04 scales
 *           work byte 0x01 by four to select its two-word handler run
 *           (0x801EEC5C, 0x801EED40), which makes this span the second arm of
 *           that run.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBIsFFBmagicMagic10703_801EED40(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0xFF) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
