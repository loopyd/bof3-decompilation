#include "bof3/bof3.h"

extern s8 D_80146872;
extern u32 D_801F6DD0[];

/* @source 0x801F6C04
 * @behavior Per-frame scenario-progress dispatcher of the scena19 overlay (entry point of its
 * 492-byte payload, offsets 0x4-0x40 = 15 instructions): it loads the shared signed
 * scenario-progress byte D_80146872 (0x80146872) with `lb`, scales that byte by four (`sll by 2`)
 * and adds the result to the overlay handler-table base 0x801F6DD0, loads the handler address
 * from that slot and tail-calls it with `jalr $v0` and no arguments, forwarding nothing and
 * returning nothing. The dispatcher therefore transfers control to whichever of this overlay's
 * progress handlers the current progress value selects - the handler entry addresses that follow
 * it in this overlay are 0x801F6C40, 0x801F6C54, 0x801F6C68, 0x801F6CA4, 0x801F6D50, 0x801F6D68
 * and 0x801F6DB4 - so the progress byte both selects the next handler and is the value those
 * handlers rewrite to move the scenario on. It reads no other game state and writes none; the
 * 0x18-byte frame exists only to save/restore $ra across the indirect call. The body is the exact
 * SCENA18 twin dispatchProgressHandler_scena18 (same 0x801F6C04 address) with a different handler
 * table base (0x801F6DD0 instead of 0x801F6D6C); every other instruction is byte-identical.
 * @status unverified
 * @match 0.00
 * @residual none
 */
void dispatchProgressHandler_scena19(void) {
  ((void (*)(void))D_801F6DD0[(s8)D_80146872])();
}
