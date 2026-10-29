#include "bof3/bof3.h"

extern u32 D_8014686C;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015DF18(u16 arg0);

/* @source 0x801F4524
 * @behavior Overlay handler: reads the shared scenario flag word D_8014686C
 * and hands it to the shared bit-set helper at 0x8015B580 with flag bit 0x3C,
 * then queues the frontend cue 0x203 through the main-exe cue dispatcher at
 * 0x8015DF18. It takes no arguments and returns nothing, and its 0x18-byte
 * frame only keeps $ra across the two calls. No in-image jal reaches the
 * address; the only image word holding it is the pointer word at vram
 * 0x801F5314 (payload 0x2714), entry 11 of the run of overlay code pointers
 * that carries the exact sibling advanceCounter2By2World03Area11913_801F41B8
 * at vram 0x801F52F0. The clear half of the same flag/cue pair,
 * clearScenarioFlagBit3CAndQueueCue203World03Area11913_801F4554, is the very
 * next word at 0x801F5318.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit3CAndQueueCue203World03Area11913_801F4524(void) {
  func_8015B580(D_8014686C, 0x3C);
  func_8015DF18(0x203);
}
