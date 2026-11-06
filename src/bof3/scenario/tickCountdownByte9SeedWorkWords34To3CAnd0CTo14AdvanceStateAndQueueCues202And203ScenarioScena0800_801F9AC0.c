#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);
/* @source 0x8015477C @kind unknown
 * Shared distance helper over the word pair passed as (x, z); callers publish
 * its result scaled by 0x10000.
 */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* @source 0x801F9AC0
 * @behavior Per-frame work-state handler for work byte 1 == 6: entry 6 of the
 * overlay handler sub-table at 0x801FE908 that
 * dispatchWorkByte1HandlerScenarioScena0800_801F95D4 indexes with that byte
 * (entry 35 of the 51-entry handler run at 0x801FE894). It decrements work byte
 * 9 modulo 256 and returns while that countdown is non-zero; on expiry it
 * re-reads the scratchpad work pointer from slot 0x44, seeds work words 0x34 and
 * 0x38 with 0x4B0000, calls the shared distance helper at 0x8015477C with word
 * 0x34 and 0x4B0000 and publishes that distance scaled by 0x10000 in work word
 * 0x3C, then seeds work words 0x0C/0x10/0x14 with
 * 0xFFFF4925/0xFFFF0000/0x4C30C4, advances work byte 1 modulo 256 and queues the
 * front-end cues 0x202 then 0x203. It takes no arguments, returns nothing and
 * the 0x18-byte frame only keeps $ra across the three calls; the field roles
 * stay offset-anchored.
 *
 * Each pointer-cell read keeps its own local at the position the original reads
 * it: the countdown read, the position seeding read and the post-call state read
 * are three separate quantities, so the position-seeding read stays block-local
 * and takes $v0 while the 0x4B0000 seed value takes $v1. Sharing one local
 * across the first two reads merges them into a single global pseudo and swaps
 * exactly that register pair.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownByte9SeedWorkWords34To3CAnd0CTo14AdvanceStateAndQueueCues202And203ScenarioScena0800_801F9AC0(
    void) {
  u8* work;
  u8* next;
  u8* later;
  u8 countdown;
  u32 state;
  s16 distance;

  work = scenarioScena0800Work;
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    next = scenarioScena0800Work;
    *(u32*)(next + 0x34) = 0x4B0000;
    *(u32*)(next + 0x38) = 0x4B0000;
    distance = func_8015477C(*(s32*)(next + 0x34), 0x4B0000);
    later = scenarioScena0800Work;
    *(u32*)(later + 0x0C) = 0xFFFF4925u;
    state = later[1];
    *(u32*)(later + 0x3C) = distance << 16;
    *(u32*)(later + 0x10) = 0xFFFF0000u;
    *(u32*)(later + 0x14) = 0x4C30C4;
    later[1] = (u8)(state + 1);
    dispatchSoundCue(0x202);
    dispatchSoundCue(0x203);
  }
}
