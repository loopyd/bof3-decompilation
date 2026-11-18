#include "bof3/bof3.h"

/* Resident reset helper of the main executable. The exact lift clearWorkFlags
 * (0x80196070 in src/bof3/ui/clearWorkFlags.c) records that it clears the
 * scratchpad work-area header bytes 0x00, 0x01, 0x02, 0x03 and 0x04.
 * @source 0x80196070
 */
extern void func_80196070(void);

/* @source 0x801D0D5C
 * @behavior Entry 3 of this overlay handler run at 0x801D1B1C, which the
 * overlay entry func_801D0C04 selects with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044 and
 * calls with no arguments: it loads that object's pointer once, subtracts
 * 0x10 from its word at +0xC and ticks its countdown byte at +0x9 down by one,
 * both through the one loaded pointer, and the word store the original leaves
 * for the guard branch delay slot. When the decremented byte reaches zero it
 * calls the resident reset helper func_80196070 with no arguments, which clears
 * the work-area header bytes 0x00 through 0x04, including the work byte at
 * +0x1 this run is dispatched on, so the run restarts at entry 0. It takes no
 * arguments, returns nothing and keeps no state across the call beyond its
 * 0x18-byte frame, which only holds $ra at 0x10; the call's delay slot stays a
 * nop.
 *
 * Evidence: the fourteen words from 0x801D1B1C name 0x801D0C70, 0x801D0CA0,
 * 0x801D0D00, 0x801D0D5C, 0x801D0DE8, 0x801D0E30, 0x801D0EC4, 0x801D0F68,
 * 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and 0x801D11A4,
 * so this body is element 3 of that run, and no `jal` and no word of this
 * payload names 0x801D0D5C, so the run alone reaches it. The byte at +0x9 is
 * the countdown the exact lift
 * tickCountdownReload16AndAdvanceWorkByte1ScenarioSce15ef200_801D0D00 ticks
 * down and re-arms, and the word at +0xC is the accumulator that this overlay
 * resets in
 * resetWorkWordCAndArmCountdown16ThenAdvanceWorkByte1ScenarioSce15ef200_801D0C70
 * and ramps by +0x10 in
 * add16ToWorkWordCAndTickCountdownReload32AdvanceWorkByte1ScenarioSce15ef200_801D0CA0.
 *
 * Shape evidence: this body is the mirror of the exact lift
 * add16ToWorkWordCAndTickCountdownReload32AdvanceWorkByte1ScenarioSce15ef200_801D0CA0
 * word for word through the guard - same pointer load, same +0x9 / +0xC
 * accesses, same `andi $v1,$v1,0xFF` pre-decrement guard and same delay-slot
 * word store - with -0x10 instead of 0x10 and a call where that sibling re-arms
 * and advances. The call, and not the mirror, is what gives this body its
 * 0x18-byte frame; its 0x48 bytes are 0x18 shorter than that sibling's 0x60.
 * A scan of every payload under out/binaries for this span's 72 bytes found no
 * second copy, so no byte-identical sibling exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void subtract16FromWorkWordCAndTickCountdownThenResetWorkStateWhenExpiredScenarioSce15ef200_801D0D5C(
    void) {
  u8* work;
  s32 value;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  value = *(s32*)(work + 0xC) - 0x10;
  *(s32*)(work + 0xC) = value;
  if (--work[0x9] == 0) {
    func_80196070();
  }
}
