#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF1CC
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 * and then invokes the shared helper at 0x801E5988. Its address is the code
 * pointer held by the table word at 0x801EEC0C, the third entry of the
 * four-word run that starts at 0x801EEC04 and that
 * dispatchWorkByte1HandlerBmagicMagic01500_801EECA0 selects on unsigned work
 * byte 1. The original materializes the halfword address once in a register
 * (the same register serves the load and the store) and keeps it live across
 * the call's delay slot, so the read-modify-write is written through a local
 * pointer; the 0x18-byte frame only keeps $ra across the call, and the 52-byte
 * body is byte-identical to the raise-and-invoke entries at 0x801EEFDC,
 * 0x801EF728, 0x801EFA40 and 0x801EFCFC of bmagic008/00. Nothing is returned
 * and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperBmagicMagic01500_801EF1CC(void) {
  u16 *flags = (u16 *)&D_801462E8;

  *flags |= 4;
  func_801E5988();
}
