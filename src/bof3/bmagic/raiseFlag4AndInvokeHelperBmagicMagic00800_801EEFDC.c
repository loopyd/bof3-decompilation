#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEFDC
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 * and then invokes the shared helper at 0x801E5988. The neighbouring
 * func_801F01E4 raises the same bit and invokes the same helper under a
 * scratchpad byte-0x0B guard; this entry does it unconditionally. The original
 * materializes the halfword address once in a register (the address is used by
 * both the load and the store) and keeps it live across the call's delay slot,
 * so the read-modify-write is written through a local pointer; the 0x18-byte
 * frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperBmagicMagic00800_801EEFDC(void) {
  u16 *flags = (u16 *)&D_801462E8;

  *flags |= 4;
  func_801E5988();
}
