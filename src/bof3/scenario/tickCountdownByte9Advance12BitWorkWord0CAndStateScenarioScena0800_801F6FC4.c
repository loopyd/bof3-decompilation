#include "bof3/bof3.h"
#include "bof3/core/slus_variables.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void claimFreeWorkRecordSeedWorkPositionAndAngleVectorScenarioScena0800_801F802C(void);
u32 func_801F7E14(void);

/* @source 0x801F6FC4
 * @behavior Call local helper 0x801F802C when shared word 0x80143E6C
 * has its low two bits clear, then call local helper 0x801F7E14 and
 * discard its result. Decrement work byte 9 modulo 256 and advance
 * work word 0x0C by 0x80 modulo 0x1000. If the decremented byte is
 * zero, reload the scratchpad work pointer and increment byte 1
 * modulo 256. Takes no arguments and returns nothing. Helper and
 * work-field roles beyond these operations remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownByte9Advance12BitWorkWord0CAndStateScenarioScena0800_801F6FC4(void) {
  u8* work;
  u8 countdown;

  if ((D_80143E6C & 3) == 0) {
    claimFreeWorkRecordSeedWorkPositionAndAngleVectorScenarioScena0800_801F802C();
  }
  func_801F7E14();
  work = scenarioScena0800Work;
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  *(u32*)(work + 0x0C) = (*(u32*)(work + 0x0C) + 0x80u) & 0xFFFu;
  if (countdown == 0) {
    scenarioScena0800Work[1]++;
  }
}
