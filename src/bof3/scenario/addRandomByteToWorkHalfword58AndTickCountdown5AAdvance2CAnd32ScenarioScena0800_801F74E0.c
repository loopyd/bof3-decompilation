#include "bof3/bof3.h"
#include <rand.h>

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void projectWorkCursorStoreScreenPairAndDrawSpritePairAndRingScenarioScena0800_801F8534(void);

/* @source 0x801F74E0
 * @behavior Call local helper 0x801F8534, then add the low byte of rand() to
 * work halfword 0x58 modulo 65536 and advance work halfword 0x2C when the low
 * two bits of work halfword 0x5A are clear. Reload the scratchpad work pointer
 * at 0x1F800044, advance work halfword 0x32 by 1 and decrement work halfword
 * 0x5A modulo 65536; when that countdown reaches zero reload it with 0x2D and
 * increment work byte 1 modulo 256. Takes no arguments and returns nothing.
 * The neighbour at 0x801F7164 has the same shape and re-arms 0x5A with 0x96
 * instead, so only that stored literal separates the two. The local helper's
 * wider role and the work fields' meanings remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addRandomByteToWorkHalfword58AndTickCountdown5AAdvance2CAnd32ScenarioScena0800_801F74E0(void) {
  s32 random;
  u8* work;
  u8* next;
  u16 countdown;
  u16 value;

  projectWorkCursorStoreScreenPairAndDrawSpritePairAndRingScenarioScena0800_801F8534();
  random = rand();
  work = scenarioScena0800Work;
  *(u16*)(work + 0x58) += random & 0xFF;
  if ((*(u16*)(work + 0x5A) & 3) == 0) {
    *(u16*)(work + 0x2C) += 1;
  }
  next = scenarioScena0800Work;
  value = *(u16*)(next + 0x32) + 1;
  countdown = *(u16*)(next + 0x5A) - 1;
  *(u16*)(next + 0x5A) = countdown;
  *(u16*)(next + 0x32) = value;
  if (countdown == 0) {
    *(u16*)(next + 0x5A) = 0x2D;
    next[1]++;
  }
}
