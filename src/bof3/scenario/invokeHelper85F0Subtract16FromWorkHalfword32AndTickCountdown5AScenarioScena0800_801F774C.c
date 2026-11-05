#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void func_801F85F0(s32 arg0, s32 arg1, s16 arg2);
void func_80196070(void);

/* @source 0x801F774C
 * @behavior Call local helper 0x801F85F0 with signed work halfwords 0x2E,
 * 0x30 and 0x32. Reload the scratchpad work pointer at 0x1F800044 after
 * that call, decrement halfword 0x5A and subtract 16 from halfword 0x32
 * modulo 65536. Store the countdown before the retreated halfword, then
 * call the shared work-flag reset helper at 0x80196070 when the countdown
 * reaches zero. Takes no arguments and returns nothing. The local helper's
 * wider role and the work fields' meanings remain unknown; the shape and
 * helper identity follow the byte-adjacent sibling 0x801F73D4, which
 * advances the same halfword by 16.
 * @status partial
 * @match 96.15
 * @residual The countdown store precedes the value subtraction instead of following it; all other instructions match.
 */
void invokeHelper85F0Subtract16FromWorkHalfword32AndTickCountdown5AScenarioScena0800_801F774C(void) {
  u8* before;
  u8* after;
  u16 countdown;
  u16 value;

  before = scenarioScena0800Work;
  func_801F85F0(*(s16*)(before + 0x2E), *(s16*)(before + 0x30),
                *(s16*)(before + 0x32));
  after = scenarioScena0800Work;
  do {
    countdown = *(u16*)(after + 0x5A) - 1;
    value = *(u16*)(after + 0x32) - 0x10;
    *(u16*)(after + 0x5A) = countdown;
    *(u16*)(after + 0x32) = value;
  } while (0);
  if (countdown == 0) {
    func_80196070();
  }
}
