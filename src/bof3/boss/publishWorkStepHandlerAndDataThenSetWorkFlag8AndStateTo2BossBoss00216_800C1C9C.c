#include "bof3/bof3.h"

typedef void (*Boss00216StepHandler)(u8 index);

typedef struct Boss00216PublishWork {
  u8 unk_00[0xE4];
  Boss00216StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss00216PublishWork;

/* The data pointer this publisher installs at +0xEC is the start of this
 * payload's 0xF4-byte data row at 0x800C1F9C, the run the Splat `textbin` row
 * places at the end of the code segment; its +0xAC word 0x800C2048 is the
 * handler table func_800C1C08 dispatches through. The second pointer this
 * publisher installs, 0x800C1FB4, is the +0x18 word of that same row: it is
 * 0x800C1FAC + 8, index 4 of the 0x0600 halfword run the row holds from +0x10
 * to +0x1F, and not a row boundary, so it is referenced as its own data symbol
 * rather than as a row start. */
extern u8 D_800C1F9C; /* @source 0x800C1F9C @kind data */
extern u8 D_800C1FB4; /* @source 0x800C1FB4 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed dispatcher at 0x800C1D00, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C2084 run, the three inert handlers at
 * 0x800C1D38, 0x800C1D40 and 0x800C1D48. */
void dispatchIndexedHandlerBossBoss00216_800C1D00(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1C9C
 * @behavior Step setup of this overlay work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1F9C into +0xEC, the indexed dispatcher
 * dispatchIndexedHandlerBossBoss00216_800C1D00 into +0xE4 and the in-row
 * pointer 0x800C1FB4 (0x800C1F9C + 0x18) into +0xE8 -- then raises bit 0x8 of
 * the record +0x104 flag word, stores 2 into the state byte at +1 of the work
 * record the main program publishes at the scratchpad pointer cell 0x1F800044
 * and calls func_8014D978. No jal names 0x800C1C9C: its only in-image
 * reference is the word 0x800C2050, entry 3 of the 0x800C2048 handler run that
 * the overlay own 0x800C1AEC publisher installs at +0xE4, so the main program
 * reaches this step through that dispatch. The body is instruction-for-
 * instruction identical to the sibling publisher at 0x800C1980 of
 * BIN/BOSS/BOSS038.EMI, which installs its own run into the same three record
 * words and raises the same flag bit, so the +0x104 flag bit 0x8 belongs to
 * this step setup and not to that payload. Nothing in this payload reads the
 * +0x104 word back. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss00216_800C1C9C(
    void) {
  Boss00216PublishWork *work = (Boss00216PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C1F9C;
  work->step_handler =
      dispatchIndexedHandlerBossBoss00216_800C1D00;
  work->step_data = &D_800C1FB4;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
