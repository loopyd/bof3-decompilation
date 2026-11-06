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

/* @source 0x801F7C08
 * @behavior Overlay per-frame work-state handler, entry 2 of the work-byte-2
 * handler table at 0x801FE8E4: it decrements the work object's countdown byte
 * at offset 0x09 modulo 256 and returns as soon as that countdown is non-zero.
 * On countdown expiry it re-reads the work pointer from scratchpad slot 0x44,
 * stores the position pair 0x640000/0x760000 in its words at offsets 0x34 and
 * 0x38, and publishes the func_8015477C distance of that pair, scaled by
 * 0x10000, in its word at offset 0x3C. It then reloads the work pointer after
 * the call, re-arms the countdown byte at offset 0x09 with 0x20, reads the
 * pointer cell a third time for the work-byte-2 dispatch state, increments that
 * state modulo 256 and queues sound cue 0x202. Takes no arguments, returns
 * nothing, and its 0x18-byte frame only keeps $ra across its two calls. Field
 * roles remain offset-anchored because their wider meanings are unknown.
 *
 * Name evidence: "tickCountdownByte9" is this target's family wording for the
 * `countdown = (u8)(work[9] - 1); work[9] = countdown;` chain followed by a
 * zero test of the masked byte, as in the exact siblings
 * tickCountdownByte9SubtractWorkWord38AndAdvanceWorkByte2ScenarioScena0800_801F7C98
 * and tickCountdownByte9SeedWorkWordsAndQueueCues202And203ScenarioScena0800_801F98E0.
 * The expiry arm is the same statement sequence as the exact neighbour
 * seedWorkDistancePairArmCountdownByte9AdvanceWorkByte2AndQueueCue202ScenarioScena0800_801F7B2C
 * and differs from it only in the two stored position words; the shared
 * distance-scaling idiom is the one that sibling records.
 *
 * Table evidence: the address is the pointer word at 0x801FE8EC, entry 2 of the
 * work-byte-2 handler sub-run at 0x801FE8E4 that
 * dispatchWorkByte2HandlerThenProcessBufferScenarioScena0800_801F7AD0 indexes
 * with the work object's byte 2. No in-image jal targets the address, so the
 * table word is its only entry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownByte9SeedWorkDistancePairArmCountdownAdvanceWorkByte2AndQueueCue202ScenarioScena0800_801F7C08(void) {
  u8* work;
  u8* next;
  u8* state;
  u8 countdown;
  s16 distance;

  work = scenarioScena0800Work;
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    work = scenarioScena0800Work;
    *(u32*)(work + 0x34) = 0x640000;
    *(u32*)(work + 0x38) = 0x760000;
    distance = func_8015477C(*(s32*)(work + 0x34), 0x760000);
    next = scenarioScena0800Work;
    next[9] = 0x20;
    state = scenarioScena0800Work;
    *(u32*)(next + 0x3C) = distance << 16;
    state[2]++;
    dispatchSoundCue(0x202);
  }
}
