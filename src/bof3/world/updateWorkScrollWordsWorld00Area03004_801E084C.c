#include "bof3/world/area03004_internal.h"

/*
 * @source 0x801E084C
 * @behavior Saves work word 0x3C and clears it around projectAndQueueActiveWork, reloads
 * the scratchpad work cursor and restores the saved word, then writes words
 * 0x40/0x44 from signed divisions using word 0x60 and, on its negative path,
 * signed halfword 0x3E. A zero word 0x60 clears both output words.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateWorkScrollWordsWorld00Area03004_801E084C(void) {
  u8* work;
  u8* initialWork;
  u32 savedWord;
  s32 word;

  initialWork = D_1F800044;
  savedWord = *(u32*)(initialWork + 0x3Cu);
  *(u32*)(initialWork + 0x3Cu) = 0;
  projectAndQueueActiveWork();

  work = D_1F800044;
  word = *(s32*)(work + 0x60u);
  *(u32*)(work + 0x3Cu) = savedWord;

  if (word < 0) {
    s32 sampledCounter;
    s32 secondCounter;
    s32 firstDivisor;
    s32 firstQuotient;
    s32 negativeDivisor;
    s32 negativeDividend;
    sampledCounter = *(s16*)(work + 0x3Eu);
    sampledCounter *= 2;
    firstDivisor = (s32)((u32)word - (u32)sampledCounter);
    negativeDividend = 0x04650000;
    firstQuotient = negativeDividend / firstDivisor;
    *(s32*)(work + 0x40u) = firstQuotient & -256;

    secondCounter = *(s16*)(work + 0x3Eu);
    negativeDivisor = *(s32*)(work + 0x60u);
    secondCounter *= 2;
    negativeDivisor = (s32)((u32)negativeDivisor - (u32)secondCounter);
    negativeDividend = negativeDividend / negativeDivisor;
    *(s32*)(work + 0x44u) = negativeDividend & -256;
  } else if (word > 0) {
    s32 positiveDivisor;
    s32 positiveDividend;
    positiveDividend = 0x04650000;
    word = positiveDividend / word;
    word &= -256;
    *(s32*)(work + 0x40u) = word;
    positiveDivisor = *(s32*)(work + 0x60u);
    positiveDividend = positiveDividend / positiveDivisor;
    *(s32*)(work + 0x44u) = positiveDividend & -256;
  } else {
    *(s32*)(work + 0x40u) = 0;
    *(s32*)(work + 0x44u) = 0;
  }
}
