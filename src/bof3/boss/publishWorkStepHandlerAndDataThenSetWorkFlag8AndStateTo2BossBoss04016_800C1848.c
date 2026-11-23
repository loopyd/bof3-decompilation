#include "bof3/bof3.h"

typedef void (*Boss04016StepHandler)(u8 index);

typedef struct Boss04016PublishWork {
  u8 unk_00[0xE4];
  Boss04016StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss04016PublishWork;

/* This payload's data row at 0x800C1B1C, the 0x8C-byte run the Splat `textbin`
 * row places after this payload's code. Its first three words are the small
 * record 0x02020101, 0x03030404 and 0x00000101, its +0xC word is 0xFFFF0600
 * followed by 0xFFFFFFFF, and the eleven words from 0x800C1B30 are the code
 * pointers this payload's 0x800C1804 state dispatcher indexes with the work
 * state byte at +1. 0x800C1B28 is that record's +0xC word, inside the row and
 * not a row boundary, so it is referenced as its own data symbol rather than as
 * a row start. */
extern u8 D_800C1B1C; /* @source 0x800C1B1C @kind data */
extern u8 D_800C1B28; /* @source 0x800C1B28 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed handler dispatcher at 0x800C18AC, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C1B60 run. */
void dispatchIndexedHandlerBossBoss04016_800C18AC(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1B1C into +0xEC, the indexed dispatcher
 * dispatchIndexedHandlerBossBoss04016_800C18AC into +0xE4 and the in-row
 * pointer 0x800C1B28 (0x800C1B1C + 0xC) into +0xE8 -- then raises bit 0x8 of
 * the record +0x104 flag word, stores 2 into the state byte at +1 of the work
 * record the main program publishes at the scratchpad pointer cell 0x1F800044
 * and calls func_8014D978. No jal names 0x800C1848: its only in-image reference
 * is the word 0x800C1B30, entry 0 of the run that this payload's state
 * dispatcher at 0x800C1804 indexes with that same state byte, so writing 2
 * moves the next dispatch to that run's entry 2, the shared handler 0x801E35D8,
 * instead of re-entering this installer. Nothing in this payload reads the
 * +0x104 word back. This payload's twin at 0x800C1940 compares equal to this
 * span word for word except the low immediate of the dispatcher
 * materialization (0x19A4 there, 0x18AC here) and was still on its `asm` row,
 * so it supplied the statement order and this span's own bytes supply every
 * literal. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss04016_800C1848(
    void) {
  Boss04016PublishWork *work = (Boss04016PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C1B1C;
  work->step_handler = dispatchIndexedHandlerBossBoss04016_800C18AC;
  work->step_data = &D_800C1B28;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
