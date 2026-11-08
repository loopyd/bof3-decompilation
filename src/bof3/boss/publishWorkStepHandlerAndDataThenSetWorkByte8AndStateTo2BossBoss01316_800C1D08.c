#include "bof3/bof3.h"

typedef void (*Boss01316StepHandler)(u8 index);

typedef struct Boss01316PublishWork {
  u8 unk_00[0xE4];
  Boss01316StepHandler step_handler;
  void *step_data;
  void *data_row;
} Boss01316PublishWork;

/* The two data pointers this publisher installs are interior words of the
 * payload's reviewed `textbin` row D_800C25D8 (0x800C25D8 through the zero word
 * 0x800C26E0). 0x800C25F0 is that row's +0x18 word and 0x800C2618 its +0x40
 * word, both inside the eighteen words of payload data the row holds before the
 * handler run that starts at +0x48 (0x800C2620), so neither is a row boundary
 * and each is referenced as its own data symbol rather than as a row start. */
extern u8 D_800C25F0; /* @source 0x800C25F0 @kind data */
extern u8 D_800C2618; /* @source 0x800C2618 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Scratchpad work-record pointer cell. The original re-loads it before each of
 * the two byte stores, so the source keeps the named symbol form and evaluates
 * it again per store; the store through the loaded record may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* This payload's byte-indexed dispatcher at 0x800C1E58, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C26D4 run it selects. */
void dispatchIndexedHandlerBossBoss01316_800C1E58(u8 index);

/* @source 0x800C1D08
 * @behavior Step setup of this overlay work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C25F0 into +0xEC, the indexed dispatcher
 * dispatchIndexedHandlerBossBoss01316_800C1E58 into +0xE4 and the in-row pointer
 * 0x800C2618 (0x800C25D8 + 0x40) into +0xE8 -- then stores 1 into the byte at
 * +8 of the work record the main program publishes at the scratchpad pointer
 * cell 0x1F800044 and 2 into that record's state byte at +1. It calls nothing:
 * the body ends on the plain return path. The two work-record loads are
 * separate, one per store, and the second store fills the return-path delay
 * slot. The state byte is the value the overlay dispatcher at
 * 0x800C1CC4 scales by four into the run based at 0x800C2698, where this
 * function is entry 0 and the word 0x800C2698 is its only in-image reference,
 * so no jal reaches it: storing 2 moves the step to that same run's entry 2,
 * the main-executable handler 0x801E35D8, instead of re-entering this setup.
 * The byte at +8 is the value the step handler at 0x800C1F24 reads from the same
 * record and passes on as (byte + 4) & 0xFF to func_8014D6B8, so storing 1 seeds
 * that step's argument. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkByte8AndStateTo2BossBoss01316_800C1D08(
    void) {
  Boss01316PublishWork *work = (Boss01316PublishWork *)D_801EB4E8;

  work->data_row = &D_800C25F0;
  work->step_handler = dispatchIndexedHandlerBossBoss01316_800C1E58;
  work->step_data = &D_800C2618;
  D_1F800044[8] = 1;
  D_1F800044[1] = 2;
}
