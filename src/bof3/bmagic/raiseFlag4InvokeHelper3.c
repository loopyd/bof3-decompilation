#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word; it lives outside this overlay, so it keeps its
 * address-qualified name. This target's generated undefined_syms_auto.txt
 * binds the address, so no target-map row is added. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEF08
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 *           and then invokes the shared helper at 0x801E5988 with no
 *           arguments. Its address is the third word (index 2) of the
 *           three-entry code pointer run at 0x801EEC04, whose words are
 *           0x801EEC84, 0x801EEE9C and 0x801EEF08; that overlay entry
 *           dispatcher copies the three words into its own 0x28-byte frame and
 *           selects one with the scratchpad work object byte at offset 0x1
 *           scaled by four. The original materializes the halfword address
 *           once in a register (unsigned halfword load, ori with 0x4, halfword
 *           store through that one register, the store filling the call delay
 *           slot) and keeps it live across the call, so the read-modify-write
 *           is written through a local pointer; the 0x18-byte frame only keeps
 *           the return address at 0x10 across the helper call. Nothing is
 *           returned and no other state is read or written. A payload compare
 *           of this 0x34-byte span against 0x801EEFDC of emi/bmagic/magic008/00
 *           came out equal in all 52 bytes, so this body transcribes the
 *           already-exact sibling raiseFlag4AndInvokeHelperBmagicMagic00800_801EEFDC
 *           unchanged, with this overlay's own symbol rows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4InvokeHelper(void) {
  u16 *flags = (u16 *)&D_801462E8;

  *flags |= 4;
  func_801E5988();
}
