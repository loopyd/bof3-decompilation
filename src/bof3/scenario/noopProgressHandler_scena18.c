#include "bof3/bof3.h"

/* @source 0x801F6CA4
 * @behavior Empty per-frame progress handler for the scena18 overlay: it returns
 * immediately and touches no state, so the stage selected by progress byte
 * D_80146872 (index 3 of the overlay's handler table at 0x801F6D6C) performs no
 * per-frame work.
 */
void noopProgressHandler_scena18(void) {
}
