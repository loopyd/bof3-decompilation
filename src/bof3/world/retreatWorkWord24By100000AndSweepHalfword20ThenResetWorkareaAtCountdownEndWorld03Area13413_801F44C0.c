#include "bof3/bof3.h"

void drawTexturedRibbonAlongWorkSpanWorld03Area13413_801F4B6C(void);
void func_80196070(void);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer for the countdown group and again for the
 * sweep group, so the fixed-address cell symbol keeps both loads. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F44C0
 * @behavior Entry 2 of the work-byte-1 handler table based at 0x801F5828,
 * dispatched by this target's exact
 * dispatchStateByte1Table5828World03Area13413_801F4344 with the state byte at
 * +1 of the work record published at the scratchpad pointer cell 0x1F800044.
 * The body decrements the record countdown byte at +0x09 and does not re-arm
 * it. On the nonzero path it reloads the cell and sweeps the halfword at +0x20
 * down by 0x200 while retreating the word at +0x24 by 0x100000 - the negative
 * of the 0x01000000 step that entry 0 at 0x801F4388 seeds there, which entry 1
 * at 0x801F4400 adds to the interpolated word at +0x1C - and then calls the
 * overlay helper at 0x801F4B6C with the store of the retreated word in the call
 * delay slot. When the countdown reaches zero it calls the shared work-area
 * reset helper at 0x80196070 instead, ending the action. Takes no arguments and
 * returns nothing; its 0x18-byte frame only holds $ra at 0x10, once per call
 * and never two calls at once. The roles of the record halfword at +0x20 and of
 * its word at +0x24 stay described by offset.
 *
 * Name evidence: the only in-image reference to 0x801F44C0 is word 2 of the
 * pointer run based at 0x801F5828, so there is no jal caller; that is the
 * dispatch shape rather than missing evidence. The retreat-then-reset role
 * comes from this body's own stores and from the two helpers it calls, and the
 * step it retreats is the one entry 0 seeds and entry 1 advances.
 *
 * Shape evidence: the countdown spelling (a u8 local assigned from the cell
 * byte minus one, stored back through the same pointer and then tested for
 * zero, which emits the andi and the branch of the original), the cell reload
 * points and the statement order follow this target's exact
 * advanceWorkWord1CClampedToWord14AndRearmCountdownThenAdvanceWorkByte1World03Area13413_801F4400
 * and clearWorkTableSeedWorkWords34To3CAnd0CTo14DispatchCue208AndAdvanceWorkByte1World03Area13413_801F4388;
 * the sweep-then-word-then-call order and the relocated-word store in the call
 * delay slot are read from this function's own original bytes, whose 0x7C-byte
 * span matches no other payload span in the shipped binaries with the two jal
 * immediates masked.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatWorkWord24By100000AndSweepHalfword20ThenResetWorkareaAtCountdownEndWorld03Area13413_801F44C0(
    void) {
  u8* work;
  u8* sweep;
  u8 countdown;

  work = D_1F800044;
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    func_80196070();
  } else {
    sweep = D_1F800044;
    FIELD_REF(u16, sweep, 0x20) -= 0x200;
    FIELD_REF(u32, sweep, 0x24) += 0xFFF00000;
    drawTexturedRibbonAlongWorkSpanWorld03Area13413_801F4B6C();
  }
}
