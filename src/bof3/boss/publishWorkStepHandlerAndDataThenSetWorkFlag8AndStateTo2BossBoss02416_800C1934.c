#include "bof3/bof3.h"

typedef void (*Boss02416StepHandler)(u8 index);

typedef struct Boss02416PublishWork {
  u8 unk_00[0xE4];
  Boss02416StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss02416PublishWork;

/* This payload's data row at 0x800C1B44, the 0x8C bytes the Splat `textbin`
 * row places at the end of the code segment: its first three words are the
 * small record 0x02820080, 0x01810383 and 0x00000080, its +0xC word is the
 * 0xFFFF0600 halfword pair followed by 0x0601FFFF, and the pointer runs it
 * holds start at 0x800C1B58, at 0x800C1B88 and at 0x800C1BC4. 0x800C1B50 is
 * that record's +0xC word, inside the row and not a row boundary, so it is
 * referenced as its own data symbol rather than as a row start. */
extern u8 D_800C1B44; /* @source 0x800C1B44 @kind data */
extern u8 D_800C1B50; /* @source 0x800C1B50 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; the step setup at 0x800C1848 of this same
 * payload reaches the record through it. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed step dispatcher at 0x800C1998, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C1BC4 run. */
void dispatchIndexedHandlerBossBoss02416_800C1998(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1934
 * @behavior Step setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1B44 into +0xEC, the indexed dispatcher
 * dispatchIndexedHandlerBossBoss02416_800C1998 into +0xE4 and the in-row
 * pointer 0x800C1B50 (0x800C1B44 + 0xC) into +0xE8 -- then raises bit 0x8 of
 * the record +0x104 flag word, stores 2 into the state byte at +1 of the work
 * record the main program publishes at the scratchpad pointer cell 0x1F800044
 * and calls func_8014D978. No jal names 0x800C1934 and no lui/addiu pair
 * materializes it: its only in-image reference is the word 0x800C1B94, which is
 * both entry 3 of the 0x800C1B88 run the indexed dispatcher at 0x800C18A0
 * selects from and entry 0 of the 0x800C1B94 run this payload's state
 * dispatcher at 0x800C18F0 indexes with that same state byte, so writing 2
 * moves the next state dispatch to that run's entry 2, the shared handler
 * 0x801E35D8, instead of re-entering this span. Nothing in this payload reads
 * the +0x104 word back and the role of its 0x8 bit is not established here.
 * The body is instruction-for-instruction identical to the exact sibling
 * publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss04016_800C1848
 * apart from the three stored immediates, and a cmp of this 0x64 span against
 * that one at offset 0x48 of BIN/BOSS/BOSS040.EMI#16 found those four bytes the
 * only difference, so that body supplied the statement order, operand forms and
 * declaration style. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss02416_800C1934(
    void) {
  Boss02416PublishWork *work = (Boss02416PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C1B44;
  work->step_handler = dispatchIndexedHandlerBossBoss02416_800C1998;
  work->step_data = &D_800C1B50;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
