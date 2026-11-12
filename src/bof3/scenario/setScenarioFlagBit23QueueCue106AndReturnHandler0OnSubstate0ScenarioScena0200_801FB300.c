#include "bof3/bof3.h"

/* Shared sub-state byte of the main executable, the latch this body tests and
 * clears.
 * @source 0x80146875 @kind data */
extern u8 D_80146875;
/* Signed shared primary state byte of the main executable, cleared at the tail.
 * @source 0x80146874 @kind data */
extern u8 D_80146874;
/* Active scenario flag word the shared bit-set helper takes as its first
 * argument.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared EXE-side helper invoked with the amount 0x32 and a zero flag word.
 * Its own role is not established, so it stays declared for its address and
 * the answer it returns is discarded here. */
void func_8016666C(s32 arg0, s32 arg1);
/* Shared scenario flag bit-set helper */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared front-end cue dispatcher */
void func_8015DF18(u16 arg0);
/* Shared overlay release helper */
void func_8015C058(void);

/* @source 0x801FB300
 * @behavior Primary-state 0x17 handler of this overlay. It reads the shared
 *   sub-state byte D_80146875 once through its address and returns at once
 *   unless that byte reads zero, so every action below runs on the entry step
 *   only. On that step it runs the shared helper func_8016666C with the amount
 *   0x32 and a zero flag, sets scenario flag bit 0x23 in the flag word
 *   D_8014686C through func_8015B580, queues the front-end cue 0x106 through
 *   func_8015DF18, releases the shared overlay selector through func_8015C058
 *   and returns the overlay to handler 0 by clearing the sub-state byte and the
 *   primary state byte D_80146874, in that order. It takes no arguments and
 *   returns nothing; its 0x18-byte frame keeps $ra and $s0, the $s0 held address
 *   serves both the latch load and the tail store of the sub-state byte while
 *   the separate primary-state store materialises the 0x8014 page in $at, the
 *   first call's second argument fills the `bnez` delay slot and the first
 *   argument fills the `jal` delay slot, and the tail `jr $ra` keeps its nop
 *   delay slot.
 *
 * Name evidence: no in-image `jal` targets the address. The only payload word
 *   equal to 0x801FB300 is at 0x801FE2F0, word 0x17 of the code-pointer run
 *   based at 0x801FE294 that
 *   dispatchPrimaryStateHandlerScenarioScena0200_801F7A60 indexes with the
 *   signed primary state byte D_80146874, and
 *   requestPrimaryState17AndClearSubstateScenarioScena0200_801FD574 stores 0x17
 *   into that same byte, so the overlay reaches this span as the handler of
 *   primary state 0x17. The sub-state and state byte pair it clears is the
 *   return-to-handler-0 idiom recorded by the scena0800 lift
 *   waitForProgressOrRecordFlagThenSetFlagBit5Or6QueueMode24AndSeedFrontSelectorContextScenarioScena0800_801FB8A0.
 *
 * Shape and lever evidence: the span is a function, not data and not a
 *   fragment - it opens with `addiu $sp,$sp,-0x18`, spills $ra and $s0, every
 *   action sits inside one `bnez`-guarded block and it ends in `jr $ra` - so no
 *   data or carve lever applies. Clone is declined on payload evidence: a scan
 *   of every payload under out/binaries for these 108 bytes returned this span
 *   at payload offset 0x4700 and nothing else, so no byte-identical sibling
 *   exists to prove and the adapter was not called.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit23QueueCue106AndReturnHandler0OnSubstate0ScenarioScena0200_801FB300(void) {
  u8* state = &D_80146875;

  if (*state != 0) {
    return;
  }
  func_8016666C(0x32, 0);
  func_8015B580(D_8014686C, 0x23);
  func_8015DF18(0x106);
  func_8015C058();
  *state = 0;
  D_80146874 = 0;
}
