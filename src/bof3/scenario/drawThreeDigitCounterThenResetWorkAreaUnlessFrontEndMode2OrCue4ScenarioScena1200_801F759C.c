#include "bof3/bof3.h"

void func_801F75EC(void);
void func_80196070(void);

/* Shared front-end mode byte in the main executable: the scenario substate
 * sequence scenario/scena04/00 stores 2 into it and then waits for it to stop
 * reading 2; this handler tests the same cell against 2.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Shared front-end cue selector halfword in the main executable, armed with the
 * values 1 to 11 by the world-front state machine; this handler tests the same
 * cell against 4.
 * @source 0x80143B90 @kind unknown
 */
extern u16 D_80143B90;

/* @source 0x801F759C
 * @behavior Overlay per-frame work handler, and the pointer word at 0x801FD180,
 * that is entry 2 of the code-pointer run based at 0x801FD178 that
 * dispatchWorkByte1HandlerScenarioScena1200_801F7174 selects with the work
 * object unsigned byte at offset 0x01 scaled by four (entry 0 is the byte-2
 * dispatcher 0x801F71B8 and entry 1 is 0x801F73CC). It first calls
 * func_801F75EC, the next subsegment at 0x801F75EC: that body formats the low
 * byte of the overlay-local halfword D_801FD288 through sprintf with the
 * overlay's own format string at 0x801F6C04, which reads "%03d", into the
 * shared text buffer D_80145AD4, arms a rectangle through func_801F765C and
 * draws that buffer at (0x95, 0x22) with colour 0 through func_8014FF0C, so
 * this call paints the overlay's three-digit counter readout. It then reads the
 * front-end mode byte D_80143BB0 and the front-end cue selector halfword
 * D_80143B90 out of the main executable and calls func_80196070, the shared
 * work-area reset that clears the scratchpad game work-area header, only when
 * the mode byte differs from 2 and the cue halfword differs from 4: the two
 * tests are one short-circuited conjunction, each compiling to beq on the same
 * skip label, and the first branch delay slot holds the second test's constant
 * (addiu $v0, $zero, 4). While a front-end screen is in mode 2 or on cue 4 the
 * overlay therefore keeps its work area and redraws the counter on every frame.
 * It takes no arguments, returns nothing and writes no state of its own; the
 * 0x18-byte frame only keeps $ra across the calls. The name records the proven
 * behaviour and the two literal tests, and the address anchor keeps it
 * target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawThreeDigitCounterThenResetWorkAreaUnlessFrontEndMode2OrCue4ScenarioScena1200_801F759C(void) {
  func_801F75EC();
  if (D_80143BB0 != 2 && D_80143B90 != 4) {
    func_80196070();
  }
}
