#include "bof3/bof3.h"

extern u8 D_80144EC0;
extern s8 D_80146872;

/* @source 0x801FA698
 * @behavior Entry 0 of this overlay's handler table at 0x801FDE10 (the table
 * the frame dispatcher func_801FA65C indexes with the signed shared scenario
 * state byte D_80146872): it clears the shared main-RAM byte D_80144EC0 and
 * sets D_80146872 to 1, so the next frame's dispatch runs entry 1 (0x801FA6B4).
 * Takes no arguments and returns nothing; the constant 1 sits in $v0 only
 * because the state store needs a register operand.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedByteAndAdvanceStateScenarioScena0700_801FA698(void) {
  D_80144EC0 = 0;
  D_80146872 = 1;
}
