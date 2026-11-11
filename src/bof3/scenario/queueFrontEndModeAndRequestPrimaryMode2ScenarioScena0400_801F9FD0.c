#include "bof3/bof3.h"

/* Shared primary mode byte of the main executable; the body arms it with 2.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* Shared unsigned mode halfword of the main executable; the body raises flag
 * 0x100 in it.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Front-end mode request of the main executable, outside this payload: it
 * consumes the caller mode id and the original sets nothing but $a0 for it. */
void func_80150224(s32 arg0);

/* @source 0x801F9FD0
 * @behavior Single call-site handler of this overlay:
 *           requestPrimaryState1AndFrontEndModeOnPackedRecordMatchScenarioScena0400_801F9DF4
 *           reaches it through the jal at 0x801F9FA0 with the mode id 2
 *           (materialised at 0x801F9F28), 3 (at 0x801F9F48) or 0x26 (at
 *           0x801F9F88) in the argument register,
 *           after writing the shared state byte D_80146874 and clearing the
 *           shared sub-state byte D_80146875, and then sign-extends the low
 *           byte of the result (sll/sra 24). It queues the low byte of its
 *           argument as a front-end mode through func_80150224 at 0x80150224,
 *           arms the shared primary mode byte D_80143BB0 with 2, raises flag
 *           0x100 in the shared unsigned halfword mode word D_80146258, and
 *           returns 1. The argument is truncated explicitly with andi 0xFF in
 *           the call delay slot, the halfword load is hoisted above the
 *           mode-byte store, the mode byte constant is materialised in $v1
 *           while $v0 carries the returned 1, and that 1 is written in the jr
 *           delay slot. The 0x18-byte frame only keeps $ra across the call.
 *           The 0x40-byte body is byte-identical to the exact lift
 *           queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C
 *           in the scena03 00 overlay, proven by one cmp of the two 64-byte
 *           spans of the 0x801F6C00-load payloads, so the statement order,
 *           operand forms and declaration style come from that sibling and the
 *           two shared cells keep its names.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0400_801F9FD0(s32 arg0) {
  func_80150224(arg0 & 0xFF);
  D_80143BB0 = 2;
  D_80146258 |= 0x100;
  return 1;
}
