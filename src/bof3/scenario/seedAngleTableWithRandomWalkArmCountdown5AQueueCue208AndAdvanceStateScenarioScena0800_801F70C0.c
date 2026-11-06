#include "bof3/bof3.h"
#include <rand.h>

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void shiftTableEntriesAndInsertHeadValueScenarioScena0800_801F81F4(u16 arg0);

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801F70C0
 * @behavior Overlay work-state handler, entry 0 of the twelve-entry
 * handler-pointer table at 0x801FE8A0 that the work-byte-1 dispatcher at
 * 0x801F707C selects with unsigned byte 1 of the scratchpad work object
 * reached through pointer slot 0x1F800044. It arms the work countdown halfword
 * at 0x5A with 0x20, clears the work halfwords at 0x32 and 0x58, and then runs
 * 0x40 passes in which it adds (rand() & 0xFF) to halfword 0x58 modulo 65536
 * and shifts that running total into the head of the overlay angle table at
 * 0x801FE9EC through shiftTableEntriesAndInsertHeadValueScenarioScena0800_801F81F4,
 * so the whole angle table is refilled with a random walk. After the passes it
 * clears halfword 0x2C, queues the front-end sound cue 0x208 and increments
 * work byte 1 modulo 256 so the next frame dispatches entry 1 at 0x801F7164.
 * The scratchpad work pointer is reloaded after every call, so each call group
 * reads slot 0x44 again. Takes no arguments and returns nothing; the 0x18-byte
 * frame keeps $ra across the two calls and $s0 for the pass counter. The
 * neighbour at 0x801F743C has the identical body and queues cue 0x202 instead,
 * so only that passed literal separates the two; the angle table's consumers
 * and the work fields' wider meanings remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedAngleTableWithRandomWalkArmCountdown5AQueueCue208AndAdvanceStateScenarioScena0800_801F70C0(void) {
  s32 random;
  u8* work;
  u8* loopWork;
  u8* cueWork;
  u8* stateWork;
  u16 value;
  u8 i;

  work = scenarioScena0800Work;
  *(u16*)(work + 0x5A) = 0x20;
  *(u16*)(work + 0x32) = 0;
  *(u16*)(work + 0x58) = 0;
  i = 0;
  do {
    i++;
    random = rand();
    loopWork = scenarioScena0800Work;
    value = *(u16*)(loopWork + 0x58) + (random & 0xFF);
    *(u16*)(loopWork + 0x58) = value;
    shiftTableEntriesAndInsertHeadValueScenarioScena0800_801F81F4(value);
  } while (i < 0x40);
  cueWork = scenarioScena0800Work;
  *(u16*)(cueWork + 0x2C) = 0;
  dispatchSoundCue(0x208);
  stateWork = scenarioScena0800Work;
  stateWork[1]++;
}
