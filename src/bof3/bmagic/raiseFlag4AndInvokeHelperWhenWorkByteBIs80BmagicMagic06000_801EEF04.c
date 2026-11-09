#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. This target's generated
 * undefined_syms_auto.txt already binds the address, so no target-map row is
 * added. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEF04
 * @behavior Work-byte-1 handler of this overlay, index 4 of the five-entry code
 *           pointer run at 0x801EEC04 (0x801EECA4, 0x801EED68, 0x801EEE00,
 *           0x801EEE84 and this address, terminated by a zero word) that the
 *           dispatcher func_801EEC34 copies into its 0x30-byte frame and
 *           selects with the scratchpad work object's byte at offset 0x1 scaled
 *           by four. It reads the scratchpad work-object pointer cell
 *           0x1F800044 once, before the 0x18-byte frame exists, and returns as
 *           soon as that object's byte at offset 0xB is not 0x80: the 0x80
 *           immediate is materialised into a register ahead of the branch, so
 *           the test is an equality on the byte just loaded, with no sign
 *           extension or mask. Only on that equality does it raise bit 0x4 of
 *           the shared battle global halfword D_801462E8 (unsigned halfword
 *           load, ori, halfword store through one address register, the store
 *           filling the call delay slot) and then invoke the shared helper
 *           func_801E5988 with no arguments. The 0x18-byte frame only keeps $ra
 *           at 0x10 across that call; nothing is returned and no other state is
 *           read or written. Statement order, operand forms and declaration
 *           style follow the exact sibling
 *           raiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic00800_801F01E4,
 *           whose 0x4C-byte body matched this span word for word except for the
 *           guard; the eighteen further payload copies of that shape all test
 *           the byte against zero, so the 0x80 discriminator appears in this
 *           target's bytes only and there was no byte-identical twin to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperWhenWorkByteBIs80BmagicMagic06000_801EEF04(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0x80) {
    u16 *flags = (u16 *)&D_801462E8;

    *flags |= 4;
    func_801E5988();
  }
}
