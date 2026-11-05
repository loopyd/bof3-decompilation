#include "bof3/bof3.h"

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801F9C00
 * @behavior Per-frame work-state handler for work byte 1 == 8: it is entry 8
 * of the overlay handler sub-table at 0x801FE908 that
 * dispatchWorkByte1HandlerScenarioScena0800_801F95D4 indexes with that byte
 * (entry 37 of the handler run at 0x801FE894). It reads the work pointer from
 * scratchpad slot 0x44, seeds work words 0x34/0x38/0x3C with
 * 0x270000/0x80000/0x8000000, clears word 0x14, advances byte 1 modulo 256 so
 * the next frame dispatches entry 9 (0x801F9C58), and queues the front-end
 * cues 0x204 then 0x205. It takes no arguments, returns nothing and the
 * 0x18-byte frame only keeps $ra across the two cue calls; the field roles stay
 * offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWordsAdvanceStateAndQueueCues204And205ScenarioScena0800_801F9C00(
    void) {
  u8* work;
  u32 state;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  state = work[1];
  *(u32*)(work + 0x34) = 0x270000;
  *(u32*)(work + 0x38) = 0x80000;
  *(u32*)(work + 0x3C) = 0x8000000;
  *(u32*)(work + 0x14) = 0;
  work[1] = state + 1;
  dispatchSoundCue(0x204);
  dispatchSoundCue(0x205);
}
