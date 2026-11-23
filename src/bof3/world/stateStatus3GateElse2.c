#include "bof3/bof3.h"
#include "world/gate_status.h"

/* Shared gate byte read by this overlay's status handlers. */
extern s8 D_801490C7;

/* Shared status halfword written by this family of handlers. */
extern u16 D_801490A8;

/* @source 0x801F33D0
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read and
 *           stores 3 into the shared status halfword D_801490A8 while that
 *           byte reads nonzero, and 2 when it reads zero. Takes no arguments,
 *           calls nothing and returns nothing: the whole body is the gate load,
 *           the branch over the two constants and the halfword store.
 *
 * Evidence: the payload span 0x801F33D0-0x801F33F7 is 0x28 bytes and ten
 * instructions with no prologue and the single epilogue `jr $ra; nop`, so it is
 * one complete leaf function and not a fragment. The load-delay slot after
 * `lb $v0,%lo(D_801490C7)` stays `nop` (the gate is read, not re-read), `bnez`
 * jumps to the store carrying `addiu $v0,$zero,3` in its taken delay slot, the
 * fall-through arm holds `addiu $v0,$zero,2`, and the store is
 * `lui $at,%hi(D_801490A8)` followed by `sh %lo(D_801490A8)($at)`. The word at
 * 0x801F5B58, entry 0 of this overlay's ascending handler table
 * 0x801F5B58-0x801F5B84, points at this span and no `jal` in the payload
 * targets it, so the body is address-taken from that table rather than called
 * and its roles stay structural.
 *
 * Name evidence: 0x801490C7 and 0x801490A8 are the shared front-end gate byte
 * and status halfword that the whole area set of gate handlers reads and
 * writes; the sibling at 0x801F33F8 of this same payload stores 0xFFFF there
 * under the same gate test, and entries 1 and 2 of the same handler table are
 * its status-publishing steps. The two published values are this function's
 * own: nonzero gate publishes 3, zero gate publishes 2.
 *
 * Shape: seeded from the world gate-status family macro
 * include/world/gate_status.h, whose body is the declared `s32 value`, the
 * signed gate load, the nonzero/zero constant pair and the truncating halfword
 * store; that macro is already exact in eighteen other area targets. This span
 * differs only in its own two constants and its two cell addresses, so the
 * clone lever does not apply: a scan of out/binaries finds the ten-word body
 * and its first eight words only in this payload at offset 0x7D0. The constants
 * and both cell addresses are written from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(stateStatus3GateElse2, D_801490C7,
                         D_801490A8, 3, 2)
