#include "bof3/bof3.h"

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801F9CE0
 * @behavior Reads the work pointer from scratchpad slot 0x44, seeds words
 * 0x34/0x38/0x3C with 0x190000/0xF0000/0x6000000, clears word 0x14,
 * advances byte 1 modulo 256, and dispatches sound cue 0x20A.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWordsAdvanceStateAndQueueCue20AScenarioScena0800_801F9CE0(void) {
  u8* work;
  u32 state;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  state = work[1];
  *(u32*)(work + 0x34) = 0x190000;
  *(u32*)(work + 0x38) = 0xF0000;
  *(u32*)(work + 0x3C) = 0x6000000;
  *(u32*)(work + 0x14) = 0;
  work[1] = state + 1;
  dispatchSoundCue(0x20A);
}
