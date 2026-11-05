#include "bof3/bof3.h"
#include <rand.h>

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
extern u8 g_ScenarioProgress;
void func_801F8534(void);

/* @source 0x801F7200
 * @behavior Call local helper 0x801F8534, then add the low byte of rand()
 * to work halfword 0x58 modulo 65536. If the shared scenario progress byte
 * is 11, set work halfword 0x5A to 32 and increment work byte 1 modulo 256.
 * The work pointer is read from scratchpad slot 0x1F800044 after both calls
 * and reused for all accesses. Takes no arguments and returns nothing.
 * Helper and halfword roles beyond these operations remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addRandomByteToWorkHalfword58AndAdvanceAtProgress11ScenarioScena0800_801F7200(void) {
  s32 random;
  u8* work;

  func_801F8534();
  random = rand();
  work = scenarioScena0800Work;
  *(u16*)(work + 0x58) += random & 0xFF;
  if (g_ScenarioProgress == 11) {
    *(u16*)(work + 0x5A) = 32;
    work[1]++;
  }
}
