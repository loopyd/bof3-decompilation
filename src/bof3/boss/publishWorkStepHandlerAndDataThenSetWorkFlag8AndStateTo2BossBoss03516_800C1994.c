#include "bof3/bof3.h"

typedef void (*Boss03516StepHandler)(u8 index);

typedef struct Boss03516PublishWork {
  u8 unk_00[0xE4];
  Boss03516StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss03516PublishWork;

/* The 0xDC-byte data row at 0x800C1D04, the run the Splat `textbin` row places
 * after this payload's code: its first ten words are the small record
 * 0x01010000, 0x02020404, four zero words, 0x06000600, 0xFFFF0601 and two
 * 0xFFFFFFFF words, and the forty-four words from 0x800C1D2C to 0x800C1DDC are
 * the code pointers this payload's state dispatcher 0x800C1804 indexes with the
 * work state byte at +1. 0x800C1D10 is that record's +0xC word, inside the row
 * and not a row boundary, and 0x800C1D24 is its +0x20 word, the first of the
 * 0xFFFFFFFF run, so both are referenced as their own data symbols rather than
 * as row starts. */
extern u8 D_800C1D10; /* @source 0x800C1D10 @kind data */
extern u8 D_800C1D24; /* @source 0x800C1D24 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's indexed handler dispatcher at 0x800C19F8, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C1D98 run. */
void func_800C19F8(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1994
 * @behavior Step setup of this overlay work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the in-row
 * pointer 0x800C1D10 (0x800C1D04 + 0xC, inside that row and not a row start)
 * into +0xEC, the indexed dispatcher func_800C19F8 into +0xE4 and the in-row
 * pointer 0x800C1D24 (0x800C1D04 + 0x20) into +0xE8 -- then raises bit 0x8 of
 * the record +0x104 flag word, stores 2 into the state byte at +1 of the work
 * record the main program publishes at the scratchpad pointer cell 0x1F800044
 * and calls func_8014D978. No jal names 0x800C1994: its only in-image reference
 * is the word 0x800C1D68, entry 15 of the 0x800C1D2C pointer run that this
 * payload's state dispatcher at 0x800C1804 indexes with that same state byte,
 * so writing 2 moves the next dispatch to that run's entry 2, the shared
 * handler 0x801E35D8, instead of re-entering this step. Nothing in this payload
 * reads the +0x104 word back. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03516_800C1994(
    void) {
  Boss03516PublishWork *work = (Boss03516PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C1D10;
  work->step_handler = func_800C19F8;
  work->step_data = &D_800C1D24;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
