#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind the
 * same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it keeps
 * its address-qualified name, which the target's generated undefined-syms map
 * binds through the row D_801462E8 = 0x801462E8. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEFA4
 * @behavior Overlay work handler, entry 2 of this overlay's leading 24-word code
 *           pointer run at 0x801EEC04: the word at 0x801EEC0C names this address,
 *           and the dispatcher at 0x801EEC64 copies the first three words of the
 *           run and invokes the one its work byte 1 selects. Raises bit 0x4 of
 *           the shared battle global halfword at 0x801462E8 with an unsigned
 *           halfword read-modify-write, then invokes the shared helper at
 *           0x801E5988 with no arguments. The original materializes the halfword
 *           address once in a register because both the load and the store use
 *           it, so the access is written through a local pointer; the halfword
 *           store fills the call delay slot, and the 0x18-byte frame only keeps
 *           $ra across the call. Takes no arguments, returns nothing and reads
 *           or writes no other state.
 * @status partial
 * @match 85.71
 * @residual Preserving volatile access keeps the halfword store before the helper call instead of its delay slot, adding one instruction.
 */
void raiseFlag4AndInvokeHelperBmagicMagic00100_801EEFA4(void) {
  volatile u16 *flags = &D_801462E8;

  *flags |= 4;
  func_801E5988();
}
