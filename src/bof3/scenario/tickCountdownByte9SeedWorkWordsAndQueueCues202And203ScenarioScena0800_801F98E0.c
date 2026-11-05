#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801F98E0
 * @behavior Overlay per-frame work-state handler, entry 3 of the work-byte-1
 * handler sub-table at 0x801FE908 (entry 32 of the 51-entry run at
 * 0x801FE894): it decrements work byte 9 modulo 256 and, only when that
 * countdown reaches zero, re-reads the scratchpad work pointer from slot 0x44
 * and seeds work words 0x34/0x38/0x3C with 0x510000/0x420000/0x2C00000, reads
 * byte 1, stores 0x800000 in word 0x14, advances that byte 1 modulo 256 and
 * queues the front-end cues 0x202 then 0x203. It takes no arguments, returns
 * nothing and the 0x18-byte frame only keeps $ra across the two cue calls; the
 * field roles stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownByte9SeedWorkWordsAndQueueCues202And203ScenarioScena0800_801F98E0(
    void) {
  u8* work;
  u8* next;
  u8 countdown;
  u32 state;

  work = scenarioScena0800Work;
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    next = scenarioScena0800Work;
    *(u32*)(next + 0x34) = 0x510000;
    *(u32*)(next + 0x38) = 0x420000;
    *(u32*)(next + 0x3C) = 0x2C00000;
    state = next[1];
    *(u32*)(next + 0x14) = 0x800000;
    next[1] = (u8)(state + 1);
    dispatchSoundCue(0x202);
    dispatchSoundCue(0x203);
  }
}
