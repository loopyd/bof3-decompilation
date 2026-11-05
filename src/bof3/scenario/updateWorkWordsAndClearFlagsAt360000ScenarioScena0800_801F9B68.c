#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void func_801F9F8C(void* words, s32 unused, s32 offset, s32 mode);
void func_80196070(void);

/* @source 0x801F9B68
 * @behavior Per-frame work-state handler for work byte 1 == 7: it is entry 7
 * of the overlay handler sub-table at 0x801FE908 that
 * dispatchWorkByte1HandlerScenarioScena0800_801F95D4 indexes with that byte
 * (entry 36 of the 51-entry handler run at 0x801FE894). It adds work word 0x0C
 * to work word 0x34, work word 0x10 to work word 0x38 and work word 0x14 to
 * work word 0x3C, subtracts 0x100000 from word 0x14, and calls the overlay
 * effect helper at 0x801F9F8C with work address 0x34 and 0x20, 8, 0. After the
 * call it reloads the scratchpad work pointer from slot 0x1F800044 and calls
 * the shared work-flag reset helper at 0x80196070 when word 0x38 reads
 * 0x360000. It takes no arguments, returns nothing, and the 0x18-byte frame
 * only holds $ra across the calls; the field roles stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateWorkWordsAndClearFlagsAt360000ScenarioScena0800_801F9B68(void) {
  u8* work;
  u8* nextWork;
  u32 first;
  u32 second;
  u32 third;

  work = scenarioScena0800Work;
  first = *(u32*)(work + 0x34) + *(u32*)(work + 0x0C);
  second = *(u32*)(work + 0x10);
  third = *(u32*)(work + 0x14);
  *(u32*)(work + 0x34) = first;
  *(u32*)(work + 0x38) += second;
  *(u32*)(work + 0x3C) += third;
  *(u32*)(work + 0x14) -= 0x100000u;
  func_801F9F8C(work + 0x34, 32, 8, 0);
  nextWork = scenarioScena0800Work;
  if (*(u32*)(nextWork + 0x38) == 0x360000u) {
    func_80196070();
  }
}
