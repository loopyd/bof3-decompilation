#include "bof3/bof3.h"

typedef void (*Boss02116StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C1CB0 data row, the 184 bytes the Splat `textbin` row places at the end
 * of the segment. 0x800C1CB0 is that row's own label; 0x800C1CC8 is its +0x18
 * word, inside the row and not a row boundary, so it is referenced as its own
 * data symbol rather than as a row start. */
extern u8 D_800C1CB0; /* @source 0x800C1CB0 @kind data */
extern u8 D_800C1CC8; /* @source 0x800C1CC8 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's step dispatcher at 0x800C198C: it masks its argument with 0xFF,
 * scales it by four and calls the entry of the 0x800C1D20 run, which is word 18
 * of the 0x800C1CB0 row's pointer run. */
void dispatchIndexedHandlerBossBoss02116_800C198C(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1860
 * @behavior Step-0 setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1CB0 into +0xEC, the step dispatcher
 * dispatchIndexedHandlerBossBoss02116_800C198C into +0xE4 and the data pointer
 * 0x800C1CC8 (0x800C1CB0 + 0x18, inside that same row) into +0xE8 -- then
 * stores 2 into the state byte at +1 of the work record the main program
 * publishes at the scratchpad pointer cell 0x1F800044 and calls func_8014D978.
 * The state byte is the value func_800C1804 indexes the 0x800C1CD8 run by, and
 * word 0 of that run is this function's own address, so writing 2 selects
 * 0x801E35D8, a main-program handler, instead of re-entering this function.
 * That data word is this function's only in-image reference and no jal in the
 * payload reaches it, so the overlay entry does. Instruction for instruction
 * identical to the exact sibling publisher at 0x800C1848 of
 * BIN/BOSS/BOSS030.EMI#16 apart from the three stored addresses, which is the
 * only difference the two payloads carry. Both prologue and return path place
 * the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetStateTo2BossBoss02116_800C1860(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1CB0;
  FIELD_REF(Boss02116StepHandler, work, 0xE4u) =
      dispatchIndexedHandlerBossBoss02116_800C198C;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1CC8;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
