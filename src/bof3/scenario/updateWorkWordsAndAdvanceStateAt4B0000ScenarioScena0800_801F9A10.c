#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void func_801F9F8C(void* words, s32 unused, s32 offset, s32 mode);

/* @source 0x801F9A10
 * @behavior Add work words 0x0C, 0x10 and 0x14 to words 0x34, 0x38
 * and 0x3C modulo 2^32, then subtract 0x200000 from word 0x14.
 * Call the local helper at 0x801F9F8C with work address 0x34, 32, 8
 * and 0. Reload the scratchpad work pointer after the call; if word
 * 0x38 equals 0x4B0000, set byte 9 to 60, reload the pointer again
 * and increment byte 1 modulo 256. Takes no arguments or return value.
 * The broader word and helper roles remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateWorkWordsAndAdvanceStateAt4B0000ScenarioScena0800_801F9A10(void) {
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
  *(u32*)(work + 0x14) -= 0x200000u;
  func_801F9F8C(work + 0x34, 32, 8, 0);
  nextWork = scenarioScena0800Work;
  if (*(u32*)(nextWork + 0x38) == 0x4B0000u) {
    nextWork[9] = 60;
    scenarioScena0800Work[1]++;
  }
}
