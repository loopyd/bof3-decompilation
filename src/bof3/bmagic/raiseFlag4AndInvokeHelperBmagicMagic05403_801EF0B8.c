#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. It lives outside this overlay,
 * so it keeps its address-qualified name; this target's generated
 * undefined_syms_auto.txt already binds the address, so no target-map row is
 * added. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF0B8
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 *           and then invokes the shared helper at 0x801E5988 with no
 *           arguments. Its address is the third word (index 2) of the
 *           four-entry code pointer run at 0x801EEC04, whose words are
 *           0x801EEC98, 0x801EF05C, 0x801EF0B8 and 0x801EF0EC; the dispatcher
 *           dispatchWorkByte1HandlerBmagicMagic05403_801EEC2C copies those four
 *           words into its 0x28-byte frame and selects one with the scratchpad
 *           work object's byte at offset 0x1 scaled by four. The original
 *           materializes the halfword address once in a register (unsigned
 *           halfword load, ori, halfword store through that one register, the
 *           store filling the call's delay slot) and keeps it live across the
 *           call, so the read-modify-write is written through a local pointer;
 *           the 0x18-byte frame only keeps $ra at 0x10 across the helper call.
 *           Nothing is returned and no other state is read or written. The
 *           0x34-byte span compared byte for byte equal to the exact sibling
 *           raiseFlag4AndInvokeHelperBmagicMagic05603_801EF17C, so this lift
 *           transcribed that body unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperBmagicMagic05403_801EF0B8(void) {
  u16 *flags = (u16 *)&D_801462E8;

  *flags |= 4;
  func_801E5988();
}
