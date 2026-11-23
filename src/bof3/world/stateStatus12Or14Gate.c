#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F49D4
 * @behavior Publishes the shared status halfword at 0x801490A8 from the shared
 *           gate byte at 0x801490C7, which is read once with `lb`: 0x12 when the
 *           gate is zero, 0x13 when it is one, and 0x14 for every other gate
 *           value. Each arm stores the halfword directly and the original
 *           cross-jumps the three identical stores into one site, so the zero
 *           arm ends in `j` over its constant and the other two arms materialise
 *           their constants around a single `bne` against a materialised 1. No
 *           frame, no call, no other store; the body ends in `jr $ra` over a nop
 *           delay slot. Takes no arguments and returns nothing.
 *
 * Name evidence: 0x801490A8 is the shared status halfword and 0x801490C7 the
 * shared gate byte that this target's own exact lifts of the neighbouring
 * handlers (setStatusFFFFWhenGateElseStatus6Or7BySharedSelector..., which publish
 * the status from the gate) read and write at those addresses, and the three
 * literals this span stores are read from its own bytes. The payload names this
 * body only from a table word: 0x801F4F4C holds 0x801F49D4 inside the script-blob
 * handler run that also names 0x801F48B4, 0x801F4920 and 0x801F4980, and no
 * in-image `jal` reaches it, so its argument roles stay structural and none are
 * taken.
 *
 * Shape: the statement order (gate load first, the three-way arm chain with the
 * gate-zero arm first, one status store per arm and no result temporary), the
 * operand forms and the declaration style were selected by measurement within
 * this body's own evidence. Spellings measured before this one: one kind of
 * result temporary overwritten by the arms reached 10/14 instructions with the
 * stored value in the gate's register, a separately declared status local reached
 * 6/14 with the gate pushed to $a0, a switch over the gate reached 10/14, and a
 * conditional-expression form reached 11/14 by inverting the first test. Storing
 * the halfword in each arm, as written here, reached 14/14 instructions and
 * 56 -> 56 bytes with no first difference.
 *
 * Clone declined: a scan of all 398 extracted payloads under out/binaries for
 * this 0x38-byte body, unmasked and again with its `lui`/`sh`/`j` absolute words
 * masked so that relocation-different siblings could match, returned only this
 * span's own offset in its own payload, so no byte-identical twin exists and the
 * shapes above are measurements of this body rather than a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatus12Or14Gate(void) {
  s32 gate;

  gate = D_801490C7;
  if (gate == 0) {
    D_801490A8 = 0x12;
  } else if (gate == 1) {
    D_801490A8 = 0x13;
  } else {
    D_801490A8 = 0x14;
  }
}
