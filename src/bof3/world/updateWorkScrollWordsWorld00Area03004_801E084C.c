#include "bof3/world/area03004_internal.h"

/*
 * @source 0x801E084C
 * @behavior Saves work word 0x3C and clears it around func_8014D290,
 * reloads the scratchpad work cursor and restores the saved word,
 * then writes words 0x40/0x44 from signed divisions using word 0x60
 * and, on its negative path, signed halfword 0x3E. A zero word 0x60
 * clears both output words.
 * @status partial
 * @match 27.78
 * @residual Complete native bytes still differ. The original 360-byte
 * interval remains retained until exact reconstruction.
 */
void updateWorkScrollWordsWorld00Area03004_801E084C(void) {
  u8* work;
  u32 savedWord;
  s32 word;
  s32 counter;
  s32 denominator;

  work = D_1F800044;
  savedWord = *(u32*)(work + 0x3Cu);
  *(u32*)(work + 0x3Cu) = 0;
  func_8014D290();

  work = D_1F800044;
  word = *(s32*)(work + 0x60u);
  *(u32*)(work + 0x3Cu) = savedWord;

  if (word < 0) {
    counter = *(s16*)(work + 0x3Eu);
    denominator = (s32)((u32)word - (u32)(counter * 2));
    *(s32*)(work + 0x40u) = (0x04650000 / denominator) & -256;

    counter = *(s16*)(work + 0x3Eu);
    word = *(s32*)(work + 0x60u);
    denominator = (s32)((u32)word - (u32)(counter * 2));
    *(s32*)(work + 0x44u) = (0x04650000 / denominator) & -256;
  } else if (word > 0) {
    *(s32*)(work + 0x40u) = (0x04650000 / word) & -256;
    word = *(s32*)(work + 0x60u);
    *(s32*)(work + 0x44u) = (0x04650000 / word) & -256;
  } else {
    *(s32*)(work + 0x40u) = 0;
    *(s32*)(work + 0x44u) = 0;
  }
}
