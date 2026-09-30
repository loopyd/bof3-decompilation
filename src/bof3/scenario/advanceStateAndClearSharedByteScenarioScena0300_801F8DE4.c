#include "bof3/bof3.h"

extern s8 D_80146872;
extern u8 D_80144EA0;

/* @source 0x801F8DE4
 * @behavior Entry 0 of the overlay's 3-pointer handler table at 0x801FCFD4
 * ({0x801F8DE4, 0x801F8E00, 0x801F92F4}): it writes 1 to the signed shared
 * scenario state byte D_80146872 and clears the shared main-RAM byte
 * D_80144EA0, then returns. The next entry of that table (0x801F8E00) ends by
 * advancing D_80146872 to 2 the same way, and in the sibling scenario overlays
 * that signed byte selects the next table entry of the frame dispatcher, so
 * this handler advances the dispatch chain from entry 0 to entry 1. Takes no
 * arguments and returns nothing; the constant 1 sits in $v0 only because the
 * byte store needs a register operand.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceStateAndClearSharedByteScenarioScena0300_801F8DE4(void) {
  D_80146872 = 1;
  D_80144EA0 = 0;
}
