#include "bof3/bof3.h"

extern u8 D_80144E98;
extern u32 g_ScenarioProgress;
extern s8 D_80146872;

/* @source 0x801F71D8
 * @behavior Entry 0 of this overlay's frame-handler table at 0x801FE288 (the
 * table the overlay's frame dispatcher func_801F719C indexes with the signed
 * shared scenario state byte D_80146872): it clears the shared main-RAM byte
 * D_80144E98, clears the shared scenario progress word g_ScenarioProgress and
 * writes 1 to D_80146872, so the next frame dispatches entry 1 (0x801F71FC).
 * Takes no arguments and returns nothing; the constant 1 sits in $v0 only
 * because the state byte store needs a register operand.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedByteAndProgressThenAdvanceStateScenarioScena0200_801F71D8(void) {
  D_80144E98 = 0;
  g_ScenarioProgress = 0;
  D_80146872 = 1;
}
