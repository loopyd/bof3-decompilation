#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name and binds through the target's generated
 * undefined-symbol table, which this payload's remaining asm spans also
 * reference. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind the
 * same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EED84
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once and
 * returns immediately when the work object's unsigned byte at offset 0xB is
 * non-zero. Otherwise it loads the shared battle byte at 0x80146384 unsigned and
 * passes it to the shared helper func_800A9820, raises bit 0x4 of the shared
 * battle global halfword at 0x801462E8 (unsigned halfword load, ori, halfword
 * store) and then invokes the shared helper func_801E5988. It takes no
 * arguments, returns nothing and writes no other state; the 0x18-byte frame only
 * keeps $ra at 0x10 across the two calls. The pointer cell is loaded before that
 * frame is created, and the halfword store re-materialises its address into $at
 * instead of keeping one base register live across the call, so the
 * raise-and-invoke part is written as a plain global access. The 96 bytes are
 * byte-identical to the exact lifts at emi/bmagic/magic106/03@0x801EEE80 and
 * emi/bmagic/magic072/03@0x801EF2DC, so statement order, operand forms and
 * declaration style are taken from that source; every literal it encodes
 * (0x1F800044, offset 0xB, 0x80146384, 0x801462E8, bit 0x4 and both call
 * targets) is the same in this payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic15903_801EED84(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
