#include "bof3/bof3.h"

/* @source 0x80144EC8 @kind data */
extern u8 scenarioScena0800StateBytes[];
extern u32 g_ScenarioProgress;
extern s8 D_80146872;

void reinitializeAllFieldRecordsAndKeepRecordZeroFlaggedScenarioScena0800_801FA32C(void);
void func_801FE12C(void);
void func_8015B580(void* bitmap, u8 bit_index);

/* @source 0x801FA2D0
 * @behavior Clears the byte at 0x80144EC8 and the scenario-progress word at
 * 0x80146864, then sets the progress-handler phase byte at 0x80146872 to 1.
 * Calls the overlay helpers at 0x801FA32C and 0x801FE12C in order, then sets
 * bit 0x43 in the bitmap beginning at state-byte offset 0x60 (0x80144F28).
 * The resident helper at 0x8015B580 selects byte (bit_index >> 3) and ORs
 * that byte with 1 << (bit_index & 7). No incoming arguments are consumed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearStateAndProgressEnterPhase1AndSetBit43ScenarioScena0800_801FA2D0(void) {
  u8* state = scenarioScena0800StateBytes;

  state[0] = 0;
  g_ScenarioProgress = 0;
  D_80146872 = 1;
  reinitializeAllFieldRecordsAndKeepRecordZeroFlaggedScenarioScena0800_801FA32C();
  func_801FE12C();
  func_8015B580(state + 0x60, 0x43);
}
