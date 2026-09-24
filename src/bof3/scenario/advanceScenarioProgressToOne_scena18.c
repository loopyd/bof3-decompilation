#include "bof3/bof3.h"

extern s8 D_80146872;

/* @source 0x801F6C40
 * @behavior Stores the immediate byte value 1 into the shared signed scenario-progress byte
 * @status exact
 * @match 100.00
 * @residual none
 * D_80146872 and returns, reading no other memory; the address is entry 0 of this overlay's
 * seven-entry four-byte per-frame progress handler table at 0x801F6D6C (0x801F6C40,
 * 0x801F6C54, 0x801F6C68, 0x801F6CA4, 0x801F6CEC, 0x801F6D04, 0x801F6D50), which
 * dispatchProgressHandler_scena18 indexes with that same byte, so this handler advances
 * scenario progress from state 0 to state 1 for the next frame. Takes no arguments and
 * returns nothing.
 */
void advanceScenarioProgressToOne_scena18(void) {
  D_80146872 = 1;
}
