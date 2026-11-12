#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;

/* Shared scenario flag word the main-executable flag tester func_8015B5D4
 * indexes for flag numbers.
 * @source 0x8014686C @kind unknown
 */
extern u32 D_8014686C;

/* Overlay-local counter readout halfword: bit 15 is the random sign
 * seedRandomSignAndAdvanceWorkByte2ScenarioScena1200_801F71FC seeds, bits 8..14
 * are the phase count this handler advances and bits 0..7 are the 0..0xC7
 * counter drawFramedThreeDigitCounterScenarioScena1200_801F75EC formats.
 * @source 0x801FD288 @kind data
 */
extern u16 D_801FD288;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015DF18(u16 arg0);
void func_80196070(void);

/* @source 0x801F7240
 * @behavior Overlay per-frame work handler and entry 1 of the code-pointer run
 * based at 0x801FD184 (the word at 0x801FD188), which is the run the byte-2
 * dispatcher dispatchWorkByte2HandlerScenarioScena1200_801F71B8 selects with
 * the work object's unsigned byte at offset 0x02, so it runs while that byte
 * reads 1; entry 0 of the same run is 0x801F71FC and entry 2 is 0x801F7348. It
 * first returns after the shared work-area reset func_80196070 when the shared
 * scenario progress byte g_ScenarioProgress reads 1. Otherwise it reads the
 * overlay-local halfword D_801FD288, whose low byte is the 0..0xC7 counter that
 * drawFramedThreeDigitCounterScenarioScena1200_801F75EC formats and whose bit
 * 15 is the random sign seedRandomSignAndAdvanceWorkByte2ScenarioScena1200_801F71FC
 * seeds: when that low byte already reads 0xC8 or more it only advances the
 * work object's unsigned byte at offset 0x02, which moves the state index of the
 * window this run is selected by, and leaves the halfword alone. With a spare
 * counter step it increments the seven-bit phase count in bits 8..14, and when
 * that incremented count reaches 0x1E with bit 15 clear, or 0x28 with bit 15
 * set, it clears the phase count, increments the counter low byte and queues
 * cue 0x20A through func_8015DF18, but only when the main-executable flag tester
 * func_8015B5D4 reports flag 0x14 set in the shared flag word D_8014686C; it
 * then repacks bit 15, the phase count and the counter low byte into
 * D_801FD288. It takes no arguments, returns nothing and reads no other state;
 * the 0x20-byte frame keeps $ra and $s0, $s1 and $s2 across the two calls, and
 * each shared-cell access materialises its own page in $at. The name records the
 * proven behavior and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounterPhaseQueueCue20AOnRolloverAndAdvanceWorkByte2WhenCounterFullScenarioScena1200_801F7240(void) {
  u16 value;
  u8 counter;
  u8 flag;
  u32 phase;
  u8 *work;

  if (g_ScenarioProgress == 1) {
    func_80196070();
    return;
  }
  value = D_801FD288;
  if ((value & 0xFF) >= 0xC8) {
    work = SPAD_PTR_SLOT(u8, 0x44u);
    work[2]++;
    return;
  }
  flag = value >> 15;
  phase = ((value & 0x7F00) >> 8) + 1;
  counter = value;
  if ((flag == 0 && phase >= 0x1E) || (flag != 0 && phase >= 0x28)) {
    phase = 0;
    counter++;
    if (func_8015B5D4(D_8014686C, 0x14) != 0) {
      func_8015DF18(0x20A);
    }
  }
  D_801FD288 = (flag << 15) | (phase << 8) | counter;
}
