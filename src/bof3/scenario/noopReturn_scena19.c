#include "bof3/bof3.h"

/* @source 0x801F6DB4
 * @behavior Empty leaf handler: the overlay's tables reference it as an inert callback
 * @status exact
 * @match 100.00
 * @residual none
 * that returns to the caller immediately without touching any state.
 */
void noopReturn_scena19(void) {
}
