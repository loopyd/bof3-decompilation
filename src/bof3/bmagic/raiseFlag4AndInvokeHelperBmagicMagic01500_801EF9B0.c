#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word; it lives outside this overlay, so it keeps its
 * address-qualified name. This target's generated undefined_syms_auto.txt
 * already binds the address, so no target-map row is added. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF9B0
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 *           and then invokes the shared helper at 0x801E5988 with no
 *           arguments. Its address is the third word (index 2) of the
 *           four-entry code pointer run at 0x801EEC24, whose words are
 *           0x801EF6B0, 0x801EF93C, 0x801EF9B0 and 0x801EF200; the dispatcher
 *           dispatchWorkByte1HandlerBmagicMagic01500_801EF644 copies those
 *           four words into its 0x28-byte frame and selects one with unsigned
 *           byte 1 of the scratchpad work object at 0x1F800044 scaled by four.
 *           The original materializes the halfword address once in a register
 *           (the same register serves the halfword load and the store) and
 *           keeps it live across the call's delay slot, so the
 *           read-modify-write is written through a local pointer; the
 *           0x18-byte frame only keeps $ra at 0x10 across the helper call.
 *           Nothing is returned and no other state is read or written. A
 *           payload cmp proved this span byte for byte the body of the
 *           already-exact siblings raiseFlag4AndInvokeHelperBmagicMagic01500_801EF1CC
 *           and raiseFlag4AndInvokeHelperBmagicMagic01500_801EF610, so this
 *           lift reused that body unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperBmagicMagic01500_801EF9B0(void) {
  u16 *flags = (u16 *)&D_801462E8;

  *flags |= 4;
  func_801E5988();
}
