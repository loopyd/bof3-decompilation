#include "bof3/bof3.h"

/* Shared front-end mode byte in the main executable: this handler waits while
 * it reads 2, and the sibling lift
 * drawThreeDigitCounterThenResetWorkAreaUnlessFrontEndMode2OrCue4ScenarioScena1200_801F759C
 * (0x801F759C of this overlay) tests the same cell against the same value.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Shared front-end cue selector halfword in the main executable, tested here
 * against 4.
 * @source 0x80143B90 @kind unknown
 */
extern u16 D_80143B90;

/* Shared scenario flag word the main-executable flag tester func_8015B5D4
 * indexes for flag numbers.
 * @source 0x8014686C @kind unknown
 */
extern u32 D_8014686C;

/* Shared mode/flag halfword of the main executable.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;

/* Shared primary state byte of the front-end state machine.
 * @source 0x80146874 @kind unknown
 */
extern u8 D_80146874;

/* Shared secondary sub-state byte of the front-end state machine.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_80196070(void);

/* @source 0x801F7518
 * @behavior Overlay per-frame work handler and the word at 0x801FD194, entry 1
 * of the code-pointer run based at 0x801FD190 that
 * dispatchWorkByte2HandlerScenarioScena1200_801F73CC (0x801F73CC) selects with
 * the work object unsigned byte at offset 0x02 scaled by four, so it is the
 * handler for that byte reading 1; the neighbours in the same run are entry 0,
 * the raw-named func_801F7410 (0x801F7410), and
 * clearFieldAdvanceStateAndQueueCue215ScenarioScena1200_801F7BEC (0x801F7BEC,
 * entry 2). That pointer word is the only little-endian word of the overlay
 * payload that holds 0x801F7518, so this span is reached from that table alone.
 * It requests primary scene state 5 with secondary sub-state 0x19, raises bit 8
 * in the mode halfword D_80146258 and then calls the shared work-area reset
 * func_80196070, but only when three tests all allow it: the front-end mode byte
 * D_80143BB0 differs from 2, the front-end cue halfword D_80143B90 differs from
 * 4, and the main-executable flag tester func_8015B5D4 reports flag 0x2E clear
 * in the flag word D_8014686C. The three tests are one short-circuited
 * conjunction, each compiling to a branch on the single shared exit label
 * .L801F758C, and the flag number 0x2E is materialised in the jal delay slot; a
 * non-zero return, mode 2 or cue 4 leaves every cell untouched. The 0x18-byte
 * frame only keeps $ra across the two calls, and each shared byte or halfword
 * store materialises the 0x8014 page in $at on its own, which is how the psyq
 * compiler emits separate shared-cell stores. Takes no arguments and returns
 * nothing. The 132 bytes of this span were compared byte for byte against the
 * overlay handler at 0x801F7348 and were equal, because that twin sits at the
 * same offset inside its own dispatch run and its relative branches therefore
 * encode identically; the twin had been lifted exact as
 * requestPrimaryState5Substate19SetModeFlagBit8ThenResetWorkAreaUnlessFrontEndMode2OrCue4OrFlag2ESetScenarioScena1200_801F7348,
 * so this body transcribes that statement order while keeping the dispatch
 * role recorded for this span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState5Substate19SetModeFlagBit8ThenResetWorkAreaUnlessFrontEndMode2OrCue4OrFlag2ESetScenarioScena1200_801F7518(void) {
  if (D_80143BB0 != 2 && D_80143B90 != 4 && func_8015B5D4(D_8014686C, 0x2E) == 0) {
    D_80146875 = 0x19;
    D_80146874 = 5;
    D_80146258 |= 0x100;
    func_80196070();
  }
}
