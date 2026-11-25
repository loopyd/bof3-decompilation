#include "bof3/bof3.h"

/* Shared helper in main RAM, outside this overlay's image. The payload only
 * calls it (jal 0x801E5988), so the generated undefined_funcs map keeps the raw
 * spelling and the address needs no map row; the prototype matches the one this
 * target's other exact lift gives this helper. */
extern void func_801E5988(void); /* @source 0x801E5988 */

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. This target's generated
 * undefined_syms_auto.txt already binds the address, so no target-map row is
 * added. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF114
 * @behavior The sixth and last word of the six-entry code pointer run at
 *           0x801EEC04 (0x801EEC94, 0x801EEE68, 0x801EEEDC, 0x801EEF80,
 *           0x801EF068 and this address), which
 *           dispatchWorkByte1Copy copies into its
 *           own 0x30-byte frame and selects with the scratchpad work object's
 *           byte at offset 0x1 scaled by four. The halfword at 0x801EEC18 is
 *           the only word in the payload that holds this address and no jal
 *           names it, so the table entry is its only reference. Unconditionally
 *           raises bit 0x4 of the shared battle global halfword at 0x801462E8
 *           and then invokes the shared helper at 0x801E5988 with no
 *           arguments: one lui/addiu pair materialises the halfword address in
 *           a register, an unsigned halfword load and ori 0x4 read-modify it,
 *           and the halfword store goes through that same register into the
 *           call's delay slot, so the pointer stays live across the call and is
 *           written through a local view. The 0x18-byte frame only keeps $ra at
 *           0x10 across the helper call; nothing is returned and no other state
 *           is read or written. The 52-byte span 0x801EF114..0x801EF147 came out
 *           equal to emi/bmagic/magic161/03 0x801EEF08 and
 *           emi/bmagic/magic008/00 0x801EEFDC under cmp of the two payloads, and
 *           both twins are already exact, so this body transcribes the exact
 *           sibling raiseFlag4AndInvokeHelperBmagicMagic00800_801EEFDC
 *           unchanged, with this overlay's own symbol rows.
 * @status partial
 * @match 85.71
 * @residual Preserving volatile access keeps the halfword store before the helper call instead of its delay slot, adding one instruction.
 */
void raiseFlagInvoke(void) {
  volatile u16 *flags = &D_801462E8;

  *flags |= 4;
  func_801E5988();
}
