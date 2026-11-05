#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void func_801F9F8C(void* words, s32 unused, s32 offset, s32 mode);
void func_80196070(void);

/* @source 0x801F9D30
 * @behavior Per-frame work-state handler for work byte 1 == 11: it is entry 11
 * of the overlay handler sub-table at 0x801FE908 that
 * dispatchWorkByte1HandlerScenarioScena0800_801F95D4 indexes with that byte
 * (entry 40 of the handler run at 0x801FE894). It adds 0x10000 to work word
 * 0x38, adds work word 0x14 into work word 0x3C, subtracts 0x200000 from word
 * 0x14, and calls the overlay effect helper at 0x801F9F8C with work address
 * 0x34 and 0x20, 8, 4. After the call it reloads the scratchpad work pointer
 * from slot 0x1F800044 and calls the shared work-flag reset helper at
 * 0x80196070 when the signed word 0x38 reads above 0x18FFFF. It takes no
 * arguments, returns nothing, and the 0x18-byte frame only holds $ra across
 * the calls; the field roles stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateWorkWordsAndClearFlagsAbove18FFFFScenarioScena0800_801F9D30(void) {
  u8* work;
  u8* nextWork;

  work = scenarioScena0800Work;
  *(u32*)(work + 0x38) += 0x10000u;
  *(u32*)(work + 0x3C) += *(u32*)(work + 0x14);
  *(u32*)(work + 0x14) -= 0x200000u;
  func_801F9F8C(work + 0x34, 0x20, 8, 4);
  nextWork = scenarioScena0800Work;
  if (*(s32*)(nextWork + 0x38) > 0x18FFFF) {
    func_80196070();
  }
}
