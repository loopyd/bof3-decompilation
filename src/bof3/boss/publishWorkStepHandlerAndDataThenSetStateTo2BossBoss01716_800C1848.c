#include "bof3/bof3.h"

typedef void (*Boss01716StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C1A70 data row, the 144 bytes the Splat `textbin` row places at the end
 * of the segment. 0x800C1A70 is that row's own label; 0x800C1A94 is its +0x24
 * word, the 0x0600-tagged block the row holds ahead of its 21-word pointer run,
 * inside the row and not a row boundary, so it is referenced as its own data
 * symbol rather than as a row start. */
extern u8 D_800C1A70; /* @source 0x800C1A70 @kind data */
extern u8 D_800C1A94; /* @source 0x800C1A94 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed dispatcher at 0x800C1964: it masks its argument
 * with 0xFF, scales it by four and calls the entry of the 0x800C1AF4 handler
 * table, which holds the three inert handlers 0x800C199C, 0x800C19A4 and
 * 0x800C19AC. */
void dispatchIndexedHandlerBossBoss01716_800C1964(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step-0 setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1A70 into +0xEC, the step dispatcher
 * dispatchIndexedHandlerBossBoss01716_800C1964 into +0xE4 and the in-row data
 * pointer 0x800C1A94 (0x800C1A70 + 0x24) into +0xE8 -- then stores 2 into the
 * state byte at +1 of the work record the main program publishes at the
 * scratchpad pointer cell 0x1F800044 and calls func_8014D978, the store sitting
 * in that call's delay slot. The state byte is the value the payload's own
 * dispatchStateByte1Table1aacBossBoss01716_800C1804 indexes its handler run at
 * 0x800C1AAC by, and word 0 of that run is this function's address, so writing
 * 2 makes the next dispatch select the main-executable handler 0x801E35D8
 * (word 2) instead of re-entering this installer. That run word is this
 * function's only in-image reference and no jal in the payload reaches it, so
 * the overlay entry calls it and its argument list is empty. Same body as the
 * exact publishers at 0x800C1848 of BIN/BOSS/BOSS002.EMI#16, BOSS008.EMI#16,
 * BOSS015.EMI#16 and BOSS030.EMI#16: this 0x58-byte span equals those spans
 * byte for byte apart from the three stored literals, each of which lies in
 * this payload's own 0x800C page. Both prologue and return path place the
 * boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetStateTo2BossBoss01716_800C1848(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1A70;
  FIELD_REF(Boss01716StepHandler, work, 0xE4u) =
      dispatchIndexedHandlerBossBoss01716_800C1964;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1A94;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
