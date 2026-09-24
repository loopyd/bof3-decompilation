#include "bof3/bof3.h"

extern s8 D_80146872;

/* @source 0x801F6C54
 * @behavior Stores the constant 2 into the overlay's signed shared progress byte
 * @status exact
 * @match 100.00
 * @residual none
 * D_80146872 and does nothing else; this is the handler the overlay runs while that
 * byte reads 1 (entry 1 of the seven-entry handler table at 0x801F6D6C, whose
 * words 0x801F6C40/0x801F6C54/0x801F6C68/0x801F6CA4/0x801F6CEC/0x801F6D04/
 * 0x801F6D50 are indexed by that byte in dispatchProgressHandler_scena18), so
 * writing 2 makes the next frame's dispatch invoke handler 0x801F6C68. Takes no
 * arguments and returns nothing.
 */
void advanceScenarioProgressTo2_scena18(void) {
  D_80146872 = 2;
}
