#include "bof3/bof3.h"

typedef void (*BossBoss05416StepHandler)(u8 index);

typedef struct BossBoss05416PublishWork {
  u8 unk_00[0xE4];
  BossBoss05416StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} BossBoss05416PublishWork;

/* This payload's data row at 0x800C2078, the 0x54 bytes the Splat `textbin`
 * row places at the end of the code segment: its first six words are the small
 * record 0x01010000, 0x01010404, 0, 0x06000600, 0xFFFF0602, 0x00000601, and the
 * twelve words from 0x800C2090 are this payload's step-handler run, which ends
 * before the three-word run at 0x800C20C0. 0x800C2084 is that row's +0xC word
 * inside the row and not a row boundary, so it is referenced as the word the
 * row itself carries. */
extern u8 D_800C2078; /* @source 0x800C2078 @kind data */
extern u8 D_800C2084; /* @source 0x800C2084 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed handler at 0x800C18E0, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C20C0 run. */
void func_800C18E0(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C187C
 * @behavior Step setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C2078 into +0xEC, the indexed handler func_800C18E0 into
 * +0xE4 and the in-row pointer 0x800C2084 (0x800C2078 + 0xC) into +0xE8 -- then
 * raises bit 0x8 of the record +0x104 flag word, stores 2 into the state byte
 * at +1 of the work record the main program publishes at the scratchpad pointer
 * cell 0x1F800044 and calls func_8014D978. No jal names 0x800C187C: its only
 * in-image reference is the word 0x800C2090, entry 0 of the twelve-word pointer
 * run 0x800C2090 through 0x800C20BC inside the 0x800C2078 data row that this
 * payload's state dispatcher dispatchStateByte1Table2090BossBoss05416_800C1838
 * indexes with that same state byte, so writing 2 moves the next dispatch to
 * that run's entry 2, the shared handler 0x801E35D8, instead of re-entering this
 * step. Nothing in this payload reads the +0x104 word back. The 0x64-byte body
 * is instruction-for-instruction identical to the exact sibling publisher
 * publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03616_800C1854,
 * which installs its own row into the same three record words, raises the same
 * flag bit and calls the same helper, and differs only in the three stored
 * addresses. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss05416_800C187C(
    void) {
  BossBoss05416PublishWork *work = (BossBoss05416PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C2078;
  work->step_handler = func_800C18E0;
  work->step_data = &D_800C2084;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
