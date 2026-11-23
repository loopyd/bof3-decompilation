#include "bof3/bof3.h"

typedef void (*Boss04916StepHandler)(u8 index);

typedef struct Boss04916PublishWork {
  u8 unk_00[0xE4];
  Boss04916StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss04916PublishWork;

/* This payload's data row at 0x800C1B1C, the 0x8C-byte run the Splat `textbin`
 * row places after this payload's code. Its first three words are the small
 * record 0x02020000, 0x06060404 and 0x00000000, its +0xC word is 0xFFFF0600
 * followed by 0xFFFF0601, the eleven words from 0x800C1B30 are the code
 * pointers this payload's 0x800C1804 state dispatcher indexes with the work
 * state byte at +1, and the word after them at 0x800C1B5C, 0x801E5288, closes
 * that run. 0x800C1B28 is the row's +0xC word, inside the row and not a row
 * boundary, so it is referenced as its own data symbol rather than as a row
 * start. */
extern u8 D_800C1B1C; /* @source 0x800C1B1C @kind data */
extern u8 D_800C1B28; /* @source 0x800C1B28 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed dispatcher at 0x800C18AC, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C1B60 handler table. */
void dispatchIndexedHandlerBossBoss04916_800C18AC(u8 index);

/* Shared main-program helper the boss archive family calls at the end of its
 * step setup; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1B1C into +0xEC, the indexed dispatcher
 * dispatchIndexedHandlerBossBoss04916_800C18AC into +0xE4 and the in-row
 * pointer 0x800C1B28 (0x800C1B1C + 0xC) into +0xE8 -- then raises bit 0x8 of
 * the record +0x104 flag word, stores 2 into the state byte at +1 of the work
 * record the main program publishes at the scratchpad pointer cell 0x1F800044
 * and calls func_8014D978. No jal names 0x800C1848: its only in-image reference
 * is the word 0x800C1B30, entry 0 of the run that this payload's 0x800C1804
 * state dispatcher indexes with that same state byte, so writing 2 moves the
 * next dispatch to that run's entry 2, the shared handler 0x801E35D8, instead
 * of re-entering this installer. Nothing in this payload reads the +0x104 word
 * back. This body keeps its 0x18-byte frame only to hold $ra at 0x10 across the
 * closing call, and both prologue and return path place the boundary.
 *
 * Shape: a byte compare of the two 936-byte payloads (offset 0x48, length 0x64)
 * proved this span equal to the span at 0x800C1848 of BIN/BOSS/BOSS040.EMI#16,
 * whose claimed source recorded a 100.00 match and carried the statement order
 * and declaration style transcribed here, with this target's own callee name
 * and this payload's own data row words written from these bytes. The clone
 * generator declined the span: it reports the unresolved external SPAD_PTR_SLOT
 * on both sources, so the twin served as the shape seed rather than through that
 * adapter. The later installer of this overlay at 0x800C1940 carries the same
 * 0x64-byte body with the 0x800C19A4 dispatcher.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss04916_800C1848(
    void) {
  Boss04916PublishWork *work = (Boss04916PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C1B1C;
  work->step_handler = dispatchIndexedHandlerBossBoss04916_800C18AC;
  work->step_data = &D_800C1B28;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
