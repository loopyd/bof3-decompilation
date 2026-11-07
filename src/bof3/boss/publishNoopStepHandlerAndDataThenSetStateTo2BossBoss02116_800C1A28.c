#include "bof3/bof3.h"

typedef void (*Boss02116StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C1CB0 data row, the 184 bytes the Splat `textbin` row places at the end
 * of the segment. 0x800C1CBC is that row's +0xC word and 0x800C1CD0 its +0x20
 * word; both sit inside the row rather than on a row boundary, so each is
 * referenced as its own data symbol instead of as a row start. */
extern u8 D_800C1CBC; /* @source 0x800C1CBC @kind data */
extern u8 D_800C1CD0; /* @source 0x800C1CD0 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* The step handler this publisher installs at +0xE4: it masks its argument with
 * 0xFF, scales it by four and calls the entry of the 0x800C1D5C run, whose
 * first three words are the no-op handlers 0x800C1AB8, 0x800C1AC0 and
 * 0x800C1AC8. */
void func_800C1A80(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1A28
 * @behavior Step-3 handler of this overlay's work state machine: index 3 of the
 * 0x800C1D20 run that the step dispatcher
 * dispatchIndexedHandlerBossBoss02116_800C198C installs, so it publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1CBC into +0xEC, the no-op-run step handler
 * func_800C1A80 into +0xE4 and the data pointer 0x800C1CD0 into +0xE8 -- then
 * stores 2 into the state byte at +1 of the work record the main program
 * publishes at the scratchpad pointer cell 0x1F800044 and calls func_8014D978,
 * handing the next step to the main-program handler 0x801E35D8 at index 2 of
 * the 0x800C1CD8 run. The 0x800C1D20 run entry at index 3 is this function's
 * only in-image reference, so the dispatcher reaches it. Instruction for
 * instruction identical to the exact sibling publisher at 0x800C1860 apart from
 * the three stored addresses, which is the only difference the two step
 * publishers carry. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishNoopStepHandlerAndDataThenSetStateTo2BossBoss02116_800C1A28(
    void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1CBC;
  FIELD_REF(Boss02116StepHandler, work, 0xE4u) = func_800C1A80;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1CD0;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
