#include "bof3/bof3.h"

/* Byte 1 of the overlay-wide step cell group whose byte 0 (0x801F294C)
 * dispatchStepHandler indexes the ten-entry handler run at 0x801DAD68 with.
 * Byte 1 selects a handler of the same run through the views entered at
 * 0x801DAD74 and 0x801DAD7C. Reached as its own symbol so the read folds to
 * %hi/%lo and the store re-materializes its base, the pair the original shows. */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */

/* @source 0x801D50AC
 * @behavior First handler of the overlay's byte-1 step group: word three of the
 *           ten-entry handler run at 0x801DAD68, reached as entry zero of the
 *           byte-1 view that dispatchStepByte1TableD74 indexes at 0x801DAD74.
 *           It requests frontend-local mode 0 through the main-exe setter
 *           func_8014ECAC and then advances the byte-1 step index at 0x801F294D
 *           by one. The call is made before the increment, nothing is returned,
 *           and no other cell is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setFrontLocalMode0AndAdvanceStepByte1(void) {
  func_8014ECAC(0);
  D_801F294D += 1;
}
