#include "bof3/bof3.h"

typedef void (*Boss03616StepHandler)(u8 index);

typedef struct Boss03616PublishWork {
  u8 unk_00[0xE4];
  Boss03616StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss03616PublishWork;

/* This payload's 0xA0-byte data row at 0x800C2020, the run the Splat `textbin`
 * row places at the end of the code segment: its first five words are the small
 * record 0x04040000, 0x02020303, 0, 0x06010600, 0xFFFFFFFF, and the
 * thirty-five words from 0x800C2034 to 0x800C20BC are code pointers that this
 * payload's 0x800C1810 state dispatcher indexes with the work byte at +1.
 * 0x800C202C is that record's +0xC word, 0x06010600, inside the row and not a
 * row boundary, so it is referenced as its own data symbol rather than as a row
 * start. */
extern u8 D_800C2020; /* @source 0x800C2020 @kind data */
extern u8 D_800C202C; /* @source 0x800C202C @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed dispatcher at 0x800C1B24, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C2074 run. */
void dispatchIndexedHandlerBossBoss03616_800C1B24(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1854
 * @behavior Step setup of this overlay work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C2020 into +0xEC, the indexed dispatcher
 * dispatchIndexedHandlerBossBoss03616_800C1B24 into +0xE4 and the in-row
 * pointer 0x800C202C (0x800C2020 + 0xC) into +0xE8 -- then raises bit 0x8 of
 * the record +0x104 flag word, stores 2 into the state byte at +1 of the work
 * record the main program publishes at the scratchpad pointer cell 0x1F800044
 * and calls func_8014D978. No jal names 0x800C1854: its only in-image
 * reference is the word 0x800C2034, entry 0 of the 0x800C2034 pointer run that
 * this payload's state dispatcher at 0x800C1810 indexes with that same state
 * byte, so writing 2 moves the next dispatch to that run's entry 2, the shared
 * handler 0x801E35D8, instead of re-entering this step. The body is
 * instruction-for-instruction identical to the exact sibling publisher
 * publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss00216_800C1C9C,
 * which installs its own row into the same three record words, raises the same
 * flag bit and calls the same helper, and differs only in the three stored
 * addresses; the +0x104 flag bit 0x8 belongs to this step setup and not to that
 * payload. Nothing in this payload reads the +0x104 word back. Both prologue
 * and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03616_800C1854(
    void) {
  Boss03616PublishWork *work = (Boss03616PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C2020;
  work->step_handler = dispatchIndexedHandlerBossBoss03616_800C1B24;
  work->step_data = &D_800C202C;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
