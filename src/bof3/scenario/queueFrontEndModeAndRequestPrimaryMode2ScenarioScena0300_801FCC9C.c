#include "bof3/bof3.h"

extern u8 D_80143BB0;
extern u16 D_80146258;

void func_80150224(s32 arg0);

/* @source 0x801FCC9C
 * @behavior Shared overlay tail called by four sibling handlers of this
 * overlay — func_801FC944 (jal at 0x801FC9AC with 0x33), func_801FC9DC
 * (0x801FCA88 with 0x12), func_801FCAB8 (0x801FCB1C with 0x1F) and
 * func_801FCB4C (0x801FCC6C with 0x0C, and 0x11 on the branch path that lands
 * there from 0x801FCC54) — each of which has just written the shared state byte
 * D_80146874 and the shared sub-state byte D_80146875 and then returns this
 * function's result sign-extended from its low byte (sll/sra 24). It queues the
 * low byte of its argument as a front-end mode through func_80150224, then arms
 * the primary mode byte D_80143BB0 with 2 and raises flag 0x100 in the shared
 * unsigned halfword mode word D_80146258, and returns 1. The argument is
 * truncated explicitly, because the callers pass their mode ids as constants
 * and func_80150224 consumes a byte; the load of D_80146258 is hoisted above
 * the mode-byte store, and the mode byte constant is materialised in $v1 while
 * $v0 carries the returned 1. The 0x18-byte frame only keeps $ra across the
 * call; no jal outside this overlay targets it.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C(s32 arg0) {
  func_80150224(arg0 & 0xFF);
  D_80143BB0 = 2;
  D_80146258 |= 0x100;
  return 1;
}
