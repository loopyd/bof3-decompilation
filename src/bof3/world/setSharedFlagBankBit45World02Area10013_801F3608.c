#include "bof3/bof3.h"

/* Base of the shared main-RAM flag bank this body hands to the shared flag
 * setter: the original materializes it as `lui $a0,0x8014` / `addiu
 * $a0,$a0,0x4F28`. This target's generated undefined-syms map binds the
 * address. */
extern u8 D_80144F28;

/* Shared bit-set helper of the main executable (byte bank, flag index), outside
 * this payload and reached from this body only. This target's generated
 * undefined-funcs map binds the address. */
void func_8015B580(void* arg0, s32 arg1);

/* @source 0x801F3608
 * @behavior Overlay wrapper: sets bit 0x45 of the shared main-RAM flag bank
 *           D_80144F28 through the shared flag setter func_8015B580 and returns
 *           nothing. No `jal` in this payload names it; the payload's script
 *           blob lists the address as the third of the nine handler words at
 *           0x801F4EF0, so the entry order comes from the blob and the
 *           parameter roles stay structural. The 0x18-byte frame keeps $ra at
 *           0x10 across the single call.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Evidence: the span 0x801F3608-0x801F362F is one complete 0x28-byte function
 * with its own `addiu $sp,-0x18` prologue and one `jr $ra` epilogue, and it
 * fills its Splat row exactly, so it is neither data nor a fragment. The
 * original words read `lui $a0,0x8014` / `addiu $a0,$a0,0x4F28` / `jal
 * 0x8015B580` with `addiu $a1,$zero,0x45` in the delay slot, so the bank base
 * and the flag index are 0x80144F28 and 0x45.
 *
 * Name evidence: the action is the one this target's exact lift of 0x801F3B20
 * records first, `func_8015B580(&D_80144F28, 0x45)`, so the name states the
 * write rather than a game-state role, which no bytes here establish. The
 * neighbouring counter steps 0x801F3630 and 0x801F3658 touch other shared
 * cells, so no shared role joins this body to them.
 *
 * Shape evidence: statement order, operand forms and declaration style come
 * from setSharedFlagBit4World02Area07714_801F5BD8, a whole-body wrapper of the
 * same 0x28-byte length whose payload span differs from this one in the single
 * `addiu $a1,$zero` word alone: a `cmp` of the two spans at payload offsets
 * 0xA08 of emi/world02/area100/13.bin and 0x2FD8 of emi/world02/area077/14.bin
 * leaves 39 of 40 bytes equal, with only 0x24050004 against 0x24050045
 * differing. That comparison is shape evidence, not clone eligibility, and a
 * scan of the payload binaries under out/binaries finds this 40-byte sequence
 * only at offset 0xA08 of this target's payload.
 */
void setSharedFlagBankBit45World02Area10013_801F3608(void) {
  func_8015B580(&D_80144F28, 0x45);
}
