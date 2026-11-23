#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword outside this overlay. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEFC4
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 *           and then invokes the shared helper at 0x801E5988 with no
 *           arguments, unconditionally. Its address is the third word (index
 *           2) of the three-entry code pointer run at 0x801EEC04, whose words
 *           are 0x801EEC7C, 0x801EEF44 and this address; the dispatcher
 *           dispatchWorkByte1HandlerBmagicMagic11103_801EEC1C copies those
 *           three words into its 0x28-byte frame and selects one with the
 *           scratchpad work object's byte at offset 0x1 scaled by four. The
 *           original materializes the halfword address once in a register
 *           (unsigned halfword load, ori, halfword store through that one
 *           register, the store filling the call's delay slot). The
 *           0x18-byte frame only keeps $ra at 0x10 across the helper call.
 *           This wrapper returns no value and makes no other direct data
 *           accesses.
 * @status partial
 * @match 85.71
 * @residual Preserving volatile access keeps the halfword store before the helper call instead of its delay slot, adding one instruction.
 */
void raiseFlag4AndInvokeHelperBmagicMagic11103_801EEFC4(void) {
  volatile u16 *flags = &D_801462E8;

  *flags |= 4;
  func_801E5988();
}
