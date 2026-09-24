#include "bof3/bof3.h"

extern s8 D_80146872;

/* @source 0x801F6C54
 * @behavior Stores the immediate byte value 2 into the shared signed scenario-progress
 * byte D_80146872 (0x80146872) and touches nothing else: one lui/li pair, one byte
 * store, then return. 0x801F6C54 is entry 1 of this overlay's seven-entry progress
 * handler pointer table at 0x801F6DD0 (0x801F6C40, 0x801F6C54, 0x801F6C68,
 * 0x801F6CA4, 0x801F6D50, 0x801F6D68, 0x801F6DB4), which the overlay's per-frame
 * dispatcher at 0x801F6C04 indexes with that same signed byte (lb 0x80146872, then
 * the table entry is jumped to with jalr), so this handler is the one that runs while
 * progress reads 1 and writing 2 makes the next frame dispatch entry 2 at 0x801F6C68.
 * Takes no arguments and returns nothing.
 * @status unverified
 * @match 0.00
 * @residual none
 */
void advanceScenarioProgressTo2_scena19(void) {
  D_80146872 = 2;
}
