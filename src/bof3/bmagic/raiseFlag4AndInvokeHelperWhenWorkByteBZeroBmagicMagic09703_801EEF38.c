#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEF38
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * and, only when the work object's byte at offset 0xB is zero, raises bit 0x4
 * of the shared battle global halfword at 0x801462E8 and then invokes the
 * shared helper at 0x801E5988 with no arguments. Its address is the second word
 * (index 1) of the two-entry handler pair that 0x801EEC04 seeds into its own
 * frame at sp+0x10 and sp+0x14 - 0x801EEC5C and this function - and dispatches
 * on the work object's byte 1; no other word in this payload holds
 * 0x801EEF38, so it has no direct jal caller. The pointer cell is loaded before
 * the 0x18-byte frame is created, that frame only keeps $ra across the helper
 * call, and the halfword address stays in one register across the
 * read-modify-write, so it is written through a local pointer. The whole body is
 * byte-identical to the exact siblings at 0x801F01E4 in emi/bmagic/magic008/00
 * and 0x801EF61C in emi/bmagic/magic131/03. Nothing is returned and no other
 * state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic09703_801EEF38(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0) {
    u16 *flags = (u16 *)&D_801462E8;

    *flags |= 4;
    func_801E5988();
  }
}
