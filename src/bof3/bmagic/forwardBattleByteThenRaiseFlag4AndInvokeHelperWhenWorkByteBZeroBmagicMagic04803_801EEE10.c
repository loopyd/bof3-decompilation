#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name and binds through the generated symbol file. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. It lives outside this overlay,
 * so it keeps its address-qualified name and binds through the generated symbol
 * file; the original reads it unsigned, ors 0x4 and stores it back. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEE10
 * @behavior Step 2 of this overlay's work sequence: the handler the dispatcher
 *           at 0x801EEC10 selects through work byte 1, which the three-entry
 *           code pointer table at 0x801EEC04 binds at entry 2. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once, before the
 *           frame is created, and returns immediately when the work object's
 *           unsigned byte at offset 0xB is non-zero. Otherwise it loads the
 *           shared battle byte at 0x80146384 unsigned and passes it to the
 *           shared helper func_800A9820, raises bit 0x4 of the shared battle
 *           global halfword at 0x801462E8 (unsigned halfword load, ori,
 *           halfword store that re-materialises its address into $at instead of
 *           keeping one base register live across the call) and then invokes
 *           the shared helper func_801E5988. It takes no arguments, returns
 *           nothing and writes no other state; the 0x18-byte frame only keeps
 *           the return address at 0x10 across the two calls. The 96 bytes at
 *           payload offset 0x210 are byte-identical, proven with cmp on the
 *           payload bytes, to the exact lifts at emi/bmagic/magic044/03@0x801EEDA0,
 *           emi/bmagic/magic218/03@0x801EEDB8, emi/bmagic/magic106/03@0x801EEE80,
 *           emi/bmagic/magic159/03@0x801EED84, emi/bmagic/magic072/03@0x801EF2DC,
 *           emi/bmagic/magic006/03@0x801EEDA4, emi/bmagic/magic069/03@0x801EEE30,
 *           emi/bmagic/magic124/03@0x801EF050, emi/bmagic/magic125/03@0x801EEE60,
 *           emi/bmagic/magic172/03@0x801EEDD8 and
 *           emi/bmagic/magic173/03@0x801EED90, so statement order, operand forms
 *           and declaration style are taken from that source; every literal the
 *           range encodes (0x1F800044, offset 0xB, 0x80146384, 0x801462E8, bit
 *           0x4 and both call targets) is the same here. The boundary comes from
 *           the overlay's own Splat segment, whose next entry starts at
 *           0x801EEE70 immediately after the jr at 0x801EEE6C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic04803_801EEE10(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
