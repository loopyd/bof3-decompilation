#include "bof3/bof3.h"

typedef void (*Boss03716StepHandler)(u8 index);

typedef struct Boss03716PublishWork {
  u8 unk_00[0xE4];
  Boss03716StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss03716PublishWork;

/* This payload's 0x50-byte data row at 0x800C1990, the run the Splat `textbin`
 * row places at the end of the code segment: its first five words are the small
 * record 0x02020000, 0x01010303, 0, 0x06000601, 0xFFFF0602, and the fifteen
 * words from 0x800C19A4 to the payload end 0x800C19E0 are code pointers that
 * this payload's 0x800C1804 state dispatcher indexes with the work byte at +1:
 * word 0 is this step-setup publisher 0x800C1848 itself, words 1 through 11
 * main-executable handlers (0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980,
 * 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8,
 * 0x801E5288) and words 12 through 14 this overlay's inert handlers 0x800C18E4,
 * 0x800C18EC and 0x800C18F4. 0x800C199C is that record's +0xC word, 0x06000601,
 * inside the row and not a row boundary, so it is referenced as its own data
 * symbol rather than as a row start. */
extern u8 D_800C1990; /* @source 0x800C1990 @kind data */
extern u8 D_800C199C; /* @source 0x800C199C @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed dispatcher at 0x800C18AC, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C19D4 run. */
void dispatchIndexedHandlerBoss03716_800C18AC(u8 index);

/* Shared main-program helper the boss archive family calls at the end of its
 * step setup; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step setup of this overlay work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1990 into +0xEC, the indexed dispatcher
 * dispatchIndexedHandlerBoss03716_800C18AC into +0xE4 and the in-row pointer
 * 0x800C199C (0x800C1990 + 0xC) into +0xE8 -- then raises bit 0x8 of the record
 * +0x104 flag word, stores 2 into the state byte at +1 of the work record the
 * main program publishes at the scratchpad pointer cell 0x1F800044 and calls
 * func_8014D978. No jal names 0x800C1848: its only in-image reference is word 0
 * of the 0x800C19A4 pointer run that this payload's state dispatcher 0x800C1804
 * indexes with that same state byte, so writing 2 moves the next dispatch to
 * that run's entry 2, the shared handler 0x801E35D8, instead of re-entering this
 * step. The body is instruction-for-instruction identical to the exact sibling
 * publisher publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03616_800C1854,
 * which installs its own row into the same three record words and takes the same
 * flag bit, and it differs from it only in the three stored addresses. The
 * 100-byte span is also byte-identical to the one at offset 0x48 of
 * BIN/BOSS/BOSS042.EMI#16, proved with cmp, whose own Splat row was still `asm`
 * when this one was lifted, so there was no source to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03716_800C1848(
    void) {
  Boss03716PublishWork *work = (Boss03716PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C1990;
  work->step_handler = dispatchIndexedHandlerBoss03716_800C18AC;
  work->step_data = &D_800C199C;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
