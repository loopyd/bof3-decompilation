#include "bof3/bof3.h"

extern u32 D_8014686C;

void func_8015B5A8(u32 arg0, s32 arg1);
void func_8015DF18(u16 arg0);

/* @source 0x801F4554
 * @behavior Overlay handler: reads the shared scenario flag word D_8014686C
 * and hands it to the shared bit-clear helper at 0x8015B5A8 with flag bit
 * 0x3C, then queues the frontend cue 0x203 through the main-exe cue dispatcher
 * at 0x8015DF18. It takes no arguments and returns nothing, and its 0x18-byte
 * frame only keeps $ra across the two calls. Its twelve instructions are the
 * instruction-for-instruction twin of the set half
 * setScenarioFlagBit3CAndQueueCue203World03Area11913_801F4524, differing only
 * in the first jal target (0x8015B5A8 instead of 0x8015B580). No in-image jal
 * reaches the address; the only image word holding it is the pointer word at
 * vram 0x801F5318 (payload 0x2718), the entry immediately after its set half
 * in the run of overlay code pointers that carries the exact sibling
 * advanceCounter2By2World03Area11913_801F41B8 at vram 0x801F52F0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearScenarioFlagBit3CAndQueueCue203World03Area11913_801F4554(void) {
  func_8015B5A8(D_8014686C, 0x3C);
  func_8015DF18(0x203);
}
