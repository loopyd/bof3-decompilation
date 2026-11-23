#include "bof3/bof3.h"

/* Shared status halfword of the main executable, outside this overlay's
 * payload. The whole setStatusFFFF handler family stores 0xFFFF through it on
 * every path, and this target's neighbours read the same word as its selector.
 * @source 0x801490A8 @kind data
 */
extern u16 D_801490A8;

/* Shared gate byte of the main executable, outside this overlay's payload. The
 * family reads it with `lb` (so the cell is s8) and branches on it before it
 * arms the front-end tuple; the exact world04 area168 lift
 * setStatusFFFFAndStateEntry3BProgress0SetSharedFlagBit8FWhenGateClearWorld04
 * Area16813_801F4920 holds the same zero test.
 * @source 0x801490C7 @kind data
 */
extern s8 D_801490C7;

/* Front-end state entry byte of the shared main executable, outside this
 * overlay's payload. The ui lifts dispatch through it and the world lifts arm
 * an entry/progress pair by storing the two bytes, as this body does.
 * @source 0x801448EB @kind data
 */
extern u8 D_801448EB;

/* Progress byte of that front-end entry.
 * @source 0x801448EC @kind data
 */
extern u8 D_801448EC;

/* Companion byte stored beside that entry and progress pair. Other paths reuse
 * it as a commu record row, so its role stays the family's stored-companion
 * byte rather than a proven one.
 * @source 0x801448ED @kind data
 */
extern u8 D_801448ED;

/* Shared front-end startup helper of the main executable, outside this
 * overlay's payload. The neighbouring handler of this target runs it before it
 * arms a front-end entry.
 * @source 0x8015C088
 */
extern void func_8015C088(void);

/* @source 0x801F34F8
 * @behavior Overlay handler named only by data: the payload word at 0x801F4138
 *           holds 0x801F34F8, entry 0 of the code-pointer run that continues at
 *           0x801F413C with 0x801F3558 and at 0x801F4140 with 0x801F3650, and
 *           no jal of this payload targets it, so the overlay loader or that run
 *           dispatcher enters it and its argument roles stay structural. It
 *           stores 0xFFFF into the shared status halfword D_801490A8 on every
 *           path and, while the shared gate byte D_801490C7 reads zero, runs the
 *           shared front-end startup helper func_8015C088 and then writes the
 *           shared front-end tuple: state entry 0xA at 0x801448EB, progress 5 at
 *           0x801448EC and 0xFF at 0x801448ED. The gate byte is read with `lb`
 *           and that load is hoisted above the status store; `bnez` jumps to the
 *           epilogue, so the gate-clear arm falls through its three byte stores
 *           into the frame restore. Each byte store materialises the 0x8014 page
 *           in $at on its own and each constant needs its own `addiu`, because
 *           none of the three stores writes zero. Takes no arguments and returns
 *           nothing; the 0x18-byte frame only keeps $ra across that one call and
 *           both delay slots stay nops, the branch slot because the call follows
 *           and the call slot because the helper takes no arguments.
 *
 * Name evidence: 0x801448EB is the state-entry byte, 0x801448EC its progress
 * byte and 0x801448ED the companion byte the setStatusFFFFAndStateEntry*
 * handlers of the world family store as one tuple, 0x801490C7 is the gate byte
 * they branch on, and 0x8015C088 is the shared front-end startup helper this
 * target's neighbouring handler 0x801F3558 runs before it arms an entry and
 * progress pair. The payload names this body only from the 0x801F4138 pointer
 * word.
 *
 * Shape: statement order, operand forms and declaration style are taken from
 * the exact lift
 * setStatusFFFFAndStateEntryAProgress5SubstateFFWhenGate3ThenPublishRecordFrom
 * GateRowWorld04Area16813_801F4764, whose gated block is this body's three
 * tuple stores after the same startup call, and its gate test is written as an
 * equality against zero the way the exact lift
 * setStatusFFFFAndStateEntryProgress5WhenGateClearWorld03Area14113_801F3AC0
 * writes the same literal tuple with no call and no frame. Those siblings are
 * shape evidence rather than clone sources: an exact and a masked scan of all
 * 398 payloads under out/binaries found this 0x60-byte body only at its own
 * offset 2296 of its own payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSubstateStartFrontend(
    void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_801448EB = 0xA;
    D_801448EC = 5;
    D_801448ED = 0xFF;
  }
}
