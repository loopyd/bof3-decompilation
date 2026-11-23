#include "bof3/bof3.h"

typedef void (*Boss03416StepHandler)(u8 index);

typedef struct Boss03416PublishWork {
  u8 unk_00[0xE4];
  Boss03416StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss03416PublishWork;

/* This overlay's reviewed data row at 0x800C26B8, the `textbin` row that ends
 * the code segment and spans 0x800C26B8 through 0x800C277B. Its first five
 * words are the small record 0x00000101, 0x02020101, 0x03030000, 0x00060006,
 * 0xFFFFFFFF, the words from 0x800C26CC to 0x800C2774 are in-image code
 * pointers and the last word, 0x800C2778, is zero. This function publishes the
 * row base into the work record. */
extern u8 D_800C26B8; /* @source 0x800C26B8 @kind data */

/* Word +0x0C of that same row, the 0x00060006 pair that closes the record the
 * row opens with: inside the row and not a row boundary, so it is declared at
 * its own address rather than as a second row. The row's own dispatchers name
 * the interior words 0x800C26CC, 0x800C26FC, 0x800C2714, 0x800C273C and
 * 0x800C276C the same way. */
extern u8 D_800C26C4; /* @source 0x800C26C4 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its overlays read back to install their step handler and data. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed step dispatcher at 0x800C1E58, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C276C run, whose entries 0 through 2 are
 * this overlay's inert handlers 0x800C1E90, 0x800C1E98 and 0x800C1EA0 and whose
 * entry 3 is zero. */
void dispatchIndexedHandlerBossBoss03416_800C1E58(u8 index);

/* Shared main-program helper the boss archive family calls at the end of its
 * step setup, reached here through one jal at 0x800C1E40. */
void func_8014D978(void);

/* @source 0x800C1DF4
 * @behavior Step setup of this overlay's byte-1 work state machine: publishes
 * three words into the current enemy work record loaded from 0x801EB4E8 -- the
 * data row pointer 0x800C26B8 into +0xEC, the indexed step dispatcher
 * dispatchIndexedHandlerBossBoss03416_800C1E58 into +0xE4 and the in-row record
 * word 0x800C26C4 into +0xE8 -- then raises bit 0x8 of the record's +0x104 flag
 * word, stores 2 into the state byte at +1 of the work record the main program
 * publishes at the scratchpad pointer cell 0x1F800044 and calls func_8014D978.
 * No jal names 0x800C1DF4 and the single word 0x800C1DF4 in this payload is the
 * one at 0x800C273C, entry 0 of the handler run the byte-1 dispatcher
 * dispatchStateByte1Table273cBossBoss03416_800C1DB0 indexes with that very state
 * byte, so writing 2 hands the next byte-1 dispatch to that run's entry 2,
 * 0x801E35D8, a main-program handler, instead of re-entering this setup. The
 * same word is entry 16 of the run based at 0x800C26FC that this payload's
 * byte-2 dispatcher indexes. Nothing in this payload reads the +0x104 flag word
 * back, so the role of the 0x8 bit is not established here.
 *
 * The body is instruction-for-instruction identical to the exact siblings
 * publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03716_800C1848
 * and publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03616_800C1854,
 * whose extracts differ from this span in three words only, the %lo immediates
 * of the three stored addresses; the %hi words agree because both rows and both
 * dispatchers sit in the 0x800C page. The statement order, the two-pointer
 * declaration style and the uninitialised `state` local assigned after the three
 * record stores follow those seeds, and every literal here is written from this
 * span's own bytes. Both prologue and return path place the boundary. A search
 * of the extracted payloads under out/binaries finds no byte-identical copy of
 * this body, so no clone applies. This overlay's own exact 0x800C1860 setup
 * fills the same three record words for a different row and dispatcher.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03416_800C1DF4(
    void) {
  Boss03416PublishWork *work = (Boss03416PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C26B8;
  work->step_handler = dispatchIndexedHandlerBossBoss03416_800C1E58;
  work->step_data = &D_800C26C4;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
