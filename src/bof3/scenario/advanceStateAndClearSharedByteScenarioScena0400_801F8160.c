#include "bof3/bof3.h"

extern s8 D_80146872;
extern u8 D_80144EA8;

/* @source 0x801F8160
 * @behavior Entry 0 of this overlay's 3-pointer handler table at 0x801FA204
 * ({0x801F8160, 0x801F817C, 0x801F863C}), which the frame dispatcher at
 * 0x801F8124 indexes with the signed shared scenario state byte D_80146872: it
 * writes 1 to that state byte and clears the main-RAM byte D_80144EA8 (eight
 * bytes above the byte the sibling scena03 overlay's state-0 handler clears),
 * then returns. The next entry of the table (0x801F817C) ends by advancing
 * D_80146872 to 2 the same way, so this handler advances the dispatch chain
 * from entry 0 to entry 1. Takes no arguments and returns nothing; the constant
 * 1 sits in $v0 only because the byte store needs a register operand.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceStateAndClearSharedByteScenarioScena0400_801F8160(void) {
  D_80146872 = 1;
  D_80144EA8 = 0;
}
