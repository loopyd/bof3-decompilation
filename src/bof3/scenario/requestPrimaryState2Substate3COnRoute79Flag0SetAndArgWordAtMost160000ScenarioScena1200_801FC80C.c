#include "bof3/bof3.h"

/* Shared route word of the main executable, tested here against 0x79; the
 * sibling lifts forwardRoute47RecordScenarioScena0300_801FCCDC and
 * seedPrimaryStateOnRecordCallback (0x801FC818) test the same halfword against
 * their own route constants.
 * @source 0x80143F00 @kind data
 */
extern u16 D_80143F00;

/* Shared scenario flag word the main-executable flag tester func_8015B5D4
 * indexes for flag numbers.
 * @source 0x8014686C @kind unknown
 */
extern u32 D_8014686C;

/* Shared primary state byte of the front-end state machine.
 * @source 0x80146874 @kind unknown
 */
extern u8 D_80146874;

/* Shared secondary sub-state byte of the front-end state machine.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FC80C
 * @behavior Overlay per-frame work handler and the word at 0x801FD1B0, entry 14
 * of the code-pointer run based at 0x801FD178 that
 * dispatchWorkByte1HandlerScenarioScena1200_801F7174 (0x801F7174) selects with
 * the work object's unsigned byte at offset 0x01 scaled by four, so it is the
 * handler for that byte reading 0x0E. The run's neighbour entries are 13
 * (0x801FD1AC, the raw-named func_801FC890) and 15 (0x801FD1B4,
 * dispatchCoordinateLookupHandlerScenarioScena1200_801FCC54), and its own
 * pointer word is the only little-endian word of the payload that holds
 * 0x801FC80C while no jal targets the address. On the arm it selects, it first
 * runs the shared front-end startup helper func_8015C088 and then requests
 * primary scene state 2 with secondary sub-state 0x3C by storing 0x3C in
 * D_80146875 and 2 in D_80146874, and returns 1. Three tests must all allow
 * that arm: the shared route word D_80143F00 reads 0x79, the main-executable
 * flag tester func_8015B5D4 reports flag 0 set in the flag word D_8014686C, and
 * the word the dispatching frame left in $a0, which the dispatcher itself never
 * writes because it only touches $v0 and $at, is at most 0x160000 under a
 * signed comparison. The three tests form one short-circuited conjunction: the
 * first two failures branch to the trailing return 0, whose zero the compiler
 * also writes in the delay slot of the third test's branch and jumps past the
 * shared block, and every path that is refused writes no cell at all. The
 * 0x18-byte frame keeps $ra and $s0 because the argument is compared only after
 * the flag call, and each shared byte store materialises the 0x8014 page in
 * $at on its own.
 *
 * Name evidence: the action words follow the accepted request names of this
 * overlay (requestPrimaryState5Substate19SetModeFlagBit8ThenResetWorkAreaUnlessFrontEndMode2OrCue4OrFlag2ESetScenarioScena1200_801F7518
 * and dispatchRecordCallbackByByte7AScenarioScena1200_801FC574), the route and
 * flag-guard wording follows the exact sibling
 * requestPrimaryState4Or5OnClearedFlag6Or7AndSetFlagBit16OnRoute46PackedWordMatchScenarioScena0800_801FE170,
 * and the route-word role of D_80143F00 plus the func_8015B5D4 flag test are
 * the accepted exact forms of seedPrimaryStateOnRecordCallback (0x801FC818).
 * The stored 0x3C and 2, the route constant 0x79 and the bound 0x160000 are
 * this span's own immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestPrimaryState2Substate3COnRoute79Flag0SetAndArgWordAtMost160000ScenarioScena1200_801FC80C(
    s32 arg0) {
  if (D_80143F00 == 0x79 && func_8015B5D4(D_8014686C, 0) != 0 && arg0 <= 0x160000) {
    func_8015C088();
    D_80146875 = 0x3C;
    D_80146874 = 2;
    return 1;
  }

  return 0;
}
