#include "bof3/bof3.h"
#include "bof3/core/slus_variables.h"

extern u8* scenarioScena0100Work; /* @source 0x1F800044 @kind unknown */
void func_801F7678(u8 value);
void func_801F7AE4(void);
s32 func_801F7C54(void);

/* @source 0x801F752C
 * @behavior Entry 2 of the work-byte-1 handler table at 0x801FE26C.
 * Calls func_801F7678 with 255, calls func_801F7AE4 only when bit 0 of
 * the shared word at 0x80143E6C is clear, then calls func_801F7C54 and
 * discards its result. Decrements work byte 9 modulo 256 and tests the
 * stored value. On zero, reloads byte 9 with 252 and increments byte 1.
 * Reloads the scratchpad work pointer after each store on that path,
 * preserving possible aliases with its pointer cell at 0x1F800044.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9Reload252AndAdvanceStateScenarioScena0100_801F752C(void) {
  func_801F7678(255);
  if ((D_80143E6C & 1) == 0) {
    func_801F7AE4();
  }
  func_801F7C54();
  if (--scenarioScena0100Work[9] == 0) {
    scenarioScena0100Work[9] = 252;
    scenarioScena0100Work[1]++;
  }
}
