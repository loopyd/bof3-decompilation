#include "bof3/bof3.h"

/* Shared gate byte this step tests. Siblings in this overlay read the same cell
 * to publish statuses 9 through 14, so it is the overlay's one gate counter. */
extern s8 D_801490C7; /* @source 0x801490C7 @kind data */

/* Shared status halfword this step publishes. */
extern u16 D_801490A8; /* @source 0x801490A8 @kind data */

/* Name evidence: this body publishes status 12 into the run of gate-status
 * steps that store 9, 10, 11, 12, 13 and 14 into D_801490A8, and it is the
 * only one of the run whose test is an equality against 2. The payload's
 * exported entry table at 0x801F3804 holds the run as consecutive words
 * 0x801F381C through 0x801F3834, with this span at slot 0x801F382C between
 * setStatusElevenUnlessGate (0x801F30F4) and setStatusThirteen (0x801F3144).
 *
 * Shape: the two adjacent exact siblings at 0x801F3144 and 0x801F316C carry
 * this body's statement order, operand forms and declaration style, differing
 * only in their compared and stored literals (1/0xD and 1/0xE) where this span
 * compares against 2 and stores 0xC. The three functions before them at
 * 0x801F30A4/0x801F30CC/0x801F30F4 test the gate byte against zero instead, so
 * they give the run its order but not this body's branch form. The siblings are
 * shape evidence, not clone eligibility, and the literals here come from this
 * span's own bytes. */

/* @source 0x801F311C
 * @behavior Loads the shared gate byte D_801490C7 and, when it equals 2,
 *           stores 0xC into the shared status halfword D_801490A8; when the
 *           gate byte differs from 2 it stores 0xFFFF into D_801490A8 instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatusTwelveTwo(void) {
  if (D_801490C7 == 2) {
    D_801490A8 = 0xC;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
