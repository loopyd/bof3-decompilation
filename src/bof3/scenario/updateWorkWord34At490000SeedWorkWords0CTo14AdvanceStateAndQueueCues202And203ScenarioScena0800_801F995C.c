#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void func_801F9F8C(void* words, s32 unused, s32 offset, s32 mode);
/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801F995C
 * @behavior Per-frame work-state handler for work byte 1 == 4: it is entry 4 of
 * the overlay handler sub-table at 0x801FE908 that
 * dispatchWorkByte1HandlerScenarioScena0800_801F95D4 indexes with that byte
 * (entry 33 of the 51-entry handler run at 0x801FE894). It reads the work
 * pointer from scratchpad slot 0x44, subtracts 0x10000 from work word 0x34,
 * adds work word 0x14 into work word 0x3C, subtracts 0x200000 from word 0x14,
 * and calls the overlay effect helper at 0x801F9F8C with work address 0x34 and
 * 0x20, 8, 0. After the call it reloads the work pointer from slot 0x1F800044
 * and, when word 0x34 reads 0x490000, seeds work words 0x0C/0x10/0x14 with
 * 0x38E3/0x10000/0xB8E38E, advances work byte 1 modulo 256 and queues the
 * front-end cues 0x202 then 0x203. It takes no arguments, returns nothing and
 * the 0x18-byte frame only keeps $ra across the three calls; the field roles
 * stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateWorkWord34At490000SeedWorkWords0CTo14AdvanceStateAndQueueCues202And203ScenarioScena0800_801F995C(
    void) {
  u8* work;
  u8* nextWork;
  u32 state;

  work = scenarioScena0800Work;
  *(u32*)(work + 0x34) -= 0x10000u;
  *(u32*)(work + 0x3C) += *(u32*)(work + 0x14);
  *(u32*)(work + 0x14) -= 0x200000u;
  func_801F9F8C(work + 0x34, 0x20, 8, 0);
  nextWork = scenarioScena0800Work;
  if (*(u32*)(nextWork + 0x34) == 0x490000u) {
    state = nextWork[1];
    *(u32*)(nextWork + 0x0C) = 0x38E3;
    *(u32*)(nextWork + 0x10) = 0x10000;
    *(u32*)(nextWork + 0x14) = 0xB8E38E;
    nextWork[1] = (u8)(state + 1);
    dispatchSoundCue(0x202);
    dispatchSoundCue(0x203);
  }
}
