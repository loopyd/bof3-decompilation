#include "bof3/bof3.h"

typedef void (*Boss05116StepHandler)(u8 index);

typedef struct Boss05116PublishWork {
  u8 unk_00[0xE4];
  Boss05116StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss05116PublishWork;

/* This payload's 0x58-byte data row at 0x800C1DB0, the run the Splat `textbin`
 * row places at the end of the code segment; its +0x14 word 0x800C1DC4 is the
 * state table the payload dispatcher at 0x800C1814 indexes with work byte +1.
 * The second pointer this publisher installs, 0x800C1DBC, is the +0xC word of
 * that same row, the record 0x01060006, and not a row boundary, so it is
 * referenced as its own data symbol rather than as a row start. */
extern u8 D_800C1DB0; /* @source 0x800C1DB0 @kind data */
extern u8 D_800C1DBC; /* @source 0x800C1DBC @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed dispatcher at 0x800C19C4, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C1DFC run, the three inert handlers at
 * 0x800C19FC, 0x800C1A04 and 0x800C1A0C. */
void func_800C19C4(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1858
 * @behavior Step setup of this overlay work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1DB0 into +0xEC, the byte-indexed dispatcher 0x800C19C4
 * into +0xE4 and the in-row pointer 0x800C1DBC (0x800C1DB0 + 0xC) into +0xE8
 * -- then raises bit 0x8 of the record +0x104 flag word, stores 2 into the
 * state byte at +1 of the work record the main program publishes at the
 * scratchpad pointer cell 0x1F800044 and calls func_8014D978. No jal names
 * 0x800C1858: its only in-image reference is the word 0x800C1DC4, entry 0 of
 * the state table the payload's dispatcher at 0x800C1814 indexes with that same
 * state byte, so writing 2 moves the next dispatch to that table's entry 2,
 * the shared handler 0x801E35D8, instead of re-entering this step. The body is
 * instruction-for-instruction identical to the exact siblings
 * publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03616_800C1854
 * and publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss00216_800C1C9C,
 * which install their own rows into the same three record words, raise the same
 * flag bit and call the same helper, and differ only in the three stored
 * addresses; the +0x104 flag bit 0x8 belongs to this step setup and not to
 * those payloads. Nothing in this payload reads the +0x104 word back. Both
 * prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss05116_800C1858(
    void) {
  Boss05116PublishWork *work = (Boss05116PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C1DB0;
  work->step_handler = func_800C19C4;
  work->step_data = &D_800C1DBC;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
