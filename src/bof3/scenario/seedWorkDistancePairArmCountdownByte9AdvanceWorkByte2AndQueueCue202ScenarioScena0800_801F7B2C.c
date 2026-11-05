#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
extern u16 scenarioScena0800Halfword800E4C1E; /* @source 0x800E4C1E @kind unknown */
/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);
/* @source 0x8015477C @kind unknown
 * Shared distance helper over the word pair passed as (x, z); callers publish
 * its result scaled by 0x10000.
 */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* @source 0x801F7B2C
 * @behavior Overlay per-frame work-state handler, entry 0 of the work-byte-2
 * handler table at 0x801FE8E4: seeds the shared halfword at 0x800E4C1E with
 * 0xC0, reads the work pointer from scratchpad slot 0x44 and stores the
 * position pair 0x6B0000/0x460000 in its words at offsets 0x34 and 0x38, then
 * publishes the func_8015477C distance of that pair, scaled by 0x10000, in its
 * word at offset 0x3C. It reloads the work pointer after the call, stores 0x20
 * in its countdown byte at offset 0x09, reads the pointer cell again for its
 * work-byte-2 dispatch state, increments that state modulo 256 and queues
 * sound cue 0x202. Takes no arguments, returns nothing, and its 0x18-byte
 * frame only keeps $ra across its two calls. Each pointer-cell read keeps its
 * own local at the position the original reads it: the byte-2 state load sits
 * before the distance store, which is why the original fills that load's delay
 * slot with the shift and store. Field roles remain offset-anchored because
 * their wider meanings are unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkDistancePairArmCountdownByte9AdvanceWorkByte2AndQueueCue202ScenarioScena0800_801F7B2C(void) {
  u8* work;
  u8* next;
  u8* state;
  s16 distance;

  scenarioScena0800Halfword800E4C1E = 0xC0;
  work = scenarioScena0800Work;
  *(u32*)(work + 0x34) = 0x6B0000;
  *(u32*)(work + 0x38) = 0x460000;
  distance = func_8015477C(*(s32*)(work + 0x34), 0x460000);
  next = scenarioScena0800Work;
  next[9] = 0x20;
  state = scenarioScena0800Work;
  *(u32*)(next + 0x3C) = distance << 16;
  state[2]++;
  dispatchSoundCue(0x202);
}
