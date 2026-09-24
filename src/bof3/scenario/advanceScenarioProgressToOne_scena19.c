#include "bof3/bof3.h"

extern s8 D_80146872;

/* @source 0x801F6C40
 * @behavior Stores the immediate byte value 1 into the shared signed scenario-progress byte
 * @status exact
 * @match 100.00
 * @residual none
 * D_80146872 (0x80146872) and returns, reading no other memory; the address is entry 0 of
 * this overlay's seven-entry four-byte per-frame progress handler table at 0x801F6DD0
 * (0x801F6C40, 0x801F6C54, 0x801F6C68, 0x801F6CA4, 0x801F6D50, 0x801F6D68, 0x801F6DB4),
 * which this overlay's per-frame dispatcher func_801F6C04 indexes with that same byte
 * (lb 0x80146872, then jalr to the selected entry), so this handler runs while scenario
 * progress reads 0 and advances it to state 1 for the next frame. Takes no arguments and
 * returns nothing.
 */
void advanceScenarioProgressToOne_scena19(void) {
  D_80146872 = 1;
}
