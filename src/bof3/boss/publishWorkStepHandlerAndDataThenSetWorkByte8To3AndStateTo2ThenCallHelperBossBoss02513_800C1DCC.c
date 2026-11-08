#include "bof3/bof3.h"

typedef void (*Boss02513StepHandler)(u8 index);

/* The two data pointers this publisher installs both point into this payload's
 * 0x800C3174 data row, the Splat `textbin` row that fills the last 0x148 bytes
 * of the segment. 0x800C3180 is the byte at that row's +0xC and 0x800C3194 the
 * byte at its +0x20, so each is referenced as its own data symbol rather than
 * as a row start. */
extern u8 D_800C3180; /* @source 0x800C3180 @kind data */
extern u8 D_800C3194; /* @source 0x800C3194 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Scratchpad work-record pointer cell. The original re-loads it before each of
 * the two byte stores, so the source keeps the named symbol form and evaluates
 * it again per store; the store through the loaded record may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* This payload's byte-indexed overlay dispatcher at 0x800C20F0: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C3294
 * handler run it selects. */
void dispatchIndexedHandlerBossBoss02513_800C20F0(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1E08, 0x800C1D1C and 0x800C1EF4 of
 * BIN/BOSS/BOSS008.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1DCC
 * @behavior Step setup of this overlay work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C3180 into +0xEC, the indexed dispatcher
 * dispatchIndexedHandlerBossBoss02513_800C20F0 into +0xE4 and the in-row
 * pointer 0x800C3194 into +0xE8 -- then stores 3 into the byte at +8 of the
 * work record the main program publishes at the scratchpad pointer cell
 * 0x1F800044, stores 2 into that record state byte at +1 and calls the shared
 * main-program helper func_8014D978. Both work-record loads are separate, one
 * per store, and the second store fills the call delay slot. The byte at +8 is
 * the value the sibling overlays read back from the same record as a step
 * counter, so storing 3 seeds that counter; storing 2 into the state byte moves
 * the step to entry 2 of the run the state selects instead of re-entering this
 * setup. No in-payload caller: neither this function's address nor the words it
 * stores appear in any word of this payload, so the main program reaches it
 * through its overlay entry point. Prologue and return path both place the
 * boundary; the body is instruction for instruction the shape of the exact
 * sibling publisher 0x800C1E08 of BIN/BOSS/BOSS008.EMI#16 apart from this
 * payload's own literals and the extra seeded counter store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkByte8To3AndStateTo2ThenCallHelperBossBoss02513_800C1DCC(
    void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C3180;
  FIELD_REF(Boss02513StepHandler, work, 0xE4u) =
      dispatchIndexedHandlerBossBoss02513_800C20F0;
  FIELD_REF(void *, work, 0xE8u) = &D_800C3194;
  D_1F800044[8] = 3;
  D_1F800044[1] = 2;
  func_8014D978();
}
